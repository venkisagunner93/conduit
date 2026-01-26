import os
from argparse import ArgumentParser, Namespace
from functools import partial

from .command import Command
from ..utils import find_packages, build_order, filter_to_target, build_package, run_in_levels


class BuildCommand(Command):
    name = "build"
    help = "Build packages"

    def add_arguments(self, parser: ArgumentParser) -> None:
        parser.add_argument("package", nargs="?", help="Package to build (default: all)")
        parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count(), help="Parallel jobs")

    def run(self, args: Namespace) -> None:
        packages = find_packages(self.root)
        order = build_order(packages)

        if args.package:
            order = filter_to_target(packages, order, args.package)

        install_dir = self.root / "install"
        build_dir = self.root / "build"

        install_dir.mkdir(parents=True, exist_ok=True)
        build_dir.mkdir(parents=True, exist_ok=True)

        task = partial(build_package, build_dir=build_dir, install_dir=install_dir)
        failed = run_in_levels(packages, order, task, args.jobs, action="Building")

        if failed:
            raise RuntimeError(f"Build failed for: {', '.join(failed)}")

        print("Build complete.")
