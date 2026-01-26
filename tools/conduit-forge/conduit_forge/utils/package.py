from dataclasses import dataclass
from pathlib import Path
import sys

if sys.version_info >= (3, 11):
    import tomllib
else:
    import tomli as tomllib


@dataclass
class Package:
    name: str
    version: str
    path: Path
    dependencies: list[str]


def find_packages(root: Path) -> dict[str, Package]:
    """Scan packages/ directory for conduit.toml files."""
    packages_dir = root / "packages"

    if not packages_dir.exists():
        raise RuntimeError(f"Packages directory not found: {packages_dir}")

    packages = {}

    for child in packages_dir.iterdir():
        if not child.is_dir():
            continue

        toml_path = child / "conduit.toml"
        if not toml_path.exists():
            continue

        with open(toml_path, "rb") as f:
            config = tomllib.load(f)

        pkg_config = config.get("package", {})
        deps_config = config.get("dependencies", {})

        name = pkg_config.get("name")
        if not name:
            raise RuntimeError(f"Package name missing in {toml_path}")

        if name in packages:
            raise RuntimeError(f"Duplicate package name: {name}")

        pkg = Package(
            name=name,
            version=pkg_config.get("version", "0.0.0"),
            path=child,
            dependencies=list(deps_config.keys()),
        )

        packages[name] = pkg

    if not packages:
        raise RuntimeError("No packages found in packages/ directory")

    return packages
