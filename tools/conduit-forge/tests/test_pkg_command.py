import pytest
from argparse import Namespace
from conduit_forge.commands.pkg import PkgCommand


@pytest.fixture
def workspace(tmp_path):
    (tmp_path / "packages").mkdir()
    return tmp_path


class TestPkgCommand:
    def test_creates_package_structure(self, workspace):
        cmd = PkgCommand(workspace)
        cmd.run(Namespace(name="my_pkg", deps=None, no_tests=False))

        pkg_dir = workspace / "packages" / "my_pkg"
        assert pkg_dir.exists()
        assert (pkg_dir / "conduit.toml").exists()
        assert (pkg_dir / "CMakeLists.txt").exists()
        assert (pkg_dir / "include" / "my_pkg" / "my_pkg.hpp").exists()
        assert (pkg_dir / "src" / "my_pkg.cpp").exists()
        assert (pkg_dir / "tests" / "my_pkg_test.cpp").exists()

    def test_no_tests_flag(self, workspace):
        cmd = PkgCommand(workspace)
        cmd.run(Namespace(name="my_pkg", deps=None, no_tests=True))

        pkg_dir = workspace / "packages" / "my_pkg"
        assert not (pkg_dir / "tests").exists()

    def test_with_dependencies(self, workspace):
        (workspace / "packages" / "core").mkdir()
        cmd = PkgCommand(workspace)
        cmd.run(Namespace(name="my_pkg", deps="core", no_tests=True))

        toml = (workspace / "packages" / "my_pkg" / "conduit.toml").read_text()
        assert "core" in toml

    def test_invalid_name_raises(self, workspace):
        cmd = PkgCommand(workspace)
        with pytest.raises(RuntimeError, match="Invalid package name"):
            cmd.run(Namespace(name="123bad", deps=None, no_tests=False))

    def test_duplicate_package_raises(self, workspace):
        cmd = PkgCommand(workspace)
        cmd.run(Namespace(name="my_pkg", deps=None, no_tests=True))
        with pytest.raises(RuntimeError, match="Package already exists"):
            cmd.run(Namespace(name="my_pkg", deps=None, no_tests=True))
