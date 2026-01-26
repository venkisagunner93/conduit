"""CMake build operations."""

import subprocess
from pathlib import Path

from .package import Package


def configure(pkg: Package, build_dir: Path, install_dir: Path) -> None:
    """Configure a package with CMake."""
    cmd = [
        "cmake",
        "-S", str(pkg.path),
        "-B", str(build_dir),
        "-G", "Ninja",
        f"-DCMAKE_PREFIX_PATH={install_dir}",
        f"-DCMAKE_INSTALL_PREFIX={install_dir}",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"CMake configure failed for {pkg.name}:\n{result.stderr}")


def build(build_dir: Path, pkg_name: str) -> None:
    """Build a configured package."""
    cmd = ["cmake", "--build", str(build_dir)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"CMake build failed for {pkg_name}:\n{result.stderr}")


def install(build_dir: Path, pkg_name: str) -> None:
    """Install a built package."""
    cmd = ["cmake", "--install", str(build_dir)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f"CMake install failed for {pkg_name}:\n{result.stderr}")


def test(build_dir: Path, pkg_name: str) -> None:
    """Run tests for a package."""
    cmd = ["ctest", "--test-dir", str(build_dir), "--output-on-failure"]
    result = subprocess.run(cmd, capture_output=True, text=True)

    if result.stdout:
        print(result.stdout)
    if result.stderr:
        print(result.stderr)

    if result.returncode != 0:
        raise RuntimeError(f"Tests failed for {pkg_name}")


def build_package(pkg: Package, build_dir: Path, install_dir: Path) -> None:
    """Full build pipeline: configure, build, install."""
    pkg_build_dir = build_dir / pkg.name
    pkg_build_dir.mkdir(parents=True, exist_ok=True)

    configure(pkg, pkg_build_dir, install_dir)
    build(pkg_build_dir, pkg.name)
    install(pkg_build_dir, pkg.name)


def test_package(pkg: Package, build_dir: Path) -> None:
    """Run tests for a package."""
    pkg_build_dir = build_dir / pkg.name

    if not pkg_build_dir.exists():
        raise RuntimeError(f"Build directory not found for {pkg.name}. Run 'conduit build' first.")

    test(pkg_build_dir, pkg.name)
