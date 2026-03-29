import pytest
from pathlib import Path
from conduit_forge.utils.package import find_packages


@pytest.fixture
def workspace(tmp_path):
    """Create a minimal workspace with packages."""
    packages_dir = tmp_path / "packages"
    packages_dir.mkdir()
    return tmp_path


def _create_package(workspace, name, version="0.1.0", deps=None):
    pkg_dir = workspace / "packages" / name
    pkg_dir.mkdir(parents=True, exist_ok=True)

    lines = [
        "[package]",
        f'name = "{name}"',
        f'version = "{version}"',
    ]
    if deps:
        lines.append("")
        lines.append("[dependencies]")
        for dep in deps:
            lines.append(f'{dep} = "*"')

    (pkg_dir / "conduit.toml").write_text("\n".join(lines) + "\n")
    return pkg_dir


class TestFindPackages:
    def test_finds_single_package(self, workspace):
        _create_package(workspace, "my_pkg")
        pkgs = find_packages(workspace)
        assert "my_pkg" in pkgs
        assert pkgs["my_pkg"].version == "0.1.0"
        assert pkgs["my_pkg"].dependencies == []

    def test_finds_package_with_deps(self, workspace):
        _create_package(workspace, "core")
        _create_package(workspace, "app", deps=["core"])
        pkgs = find_packages(workspace)
        assert pkgs["app"].dependencies == ["core"]

    def test_no_packages_dir_raises(self, tmp_path):
        with pytest.raises(RuntimeError, match="Packages directory not found"):
            find_packages(tmp_path)

    def test_empty_packages_dir_raises(self, workspace):
        with pytest.raises(RuntimeError, match="No packages found"):
            find_packages(workspace)

    def test_skips_non_directories(self, workspace):
        (workspace / "packages" / "not_a_package.txt").write_text("hello")
        _create_package(workspace, "real_pkg")
        pkgs = find_packages(workspace)
        assert len(pkgs) == 1
        assert "real_pkg" in pkgs

    def test_skips_dirs_without_toml(self, workspace):
        (workspace / "packages" / "no_toml").mkdir()
        _create_package(workspace, "has_toml")
        pkgs = find_packages(workspace)
        assert len(pkgs) == 1

    def test_missing_name_raises(self, workspace):
        pkg_dir = workspace / "packages" / "bad"
        pkg_dir.mkdir(parents=True)
        (pkg_dir / "conduit.toml").write_text("[package]\nversion = \"1.0\"\n")
        with pytest.raises(RuntimeError, match="Package name missing"):
            find_packages(workspace)

    def test_duplicate_name_raises(self, workspace):
        _create_package(workspace, "dup")
        # Create a second directory with the same package name in toml
        pkg_dir = workspace / "packages" / "dup_alias"
        pkg_dir.mkdir(parents=True)
        (pkg_dir / "conduit.toml").write_text('[package]\nname = "dup"\n')
        with pytest.raises(RuntimeError, match="Duplicate package name"):
            find_packages(workspace)
