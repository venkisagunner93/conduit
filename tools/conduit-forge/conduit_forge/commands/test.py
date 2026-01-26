import os
from argparse import ArgumentParser, Namespace
from functools import partial

from .command import Command
from ..utils import find_packages, build_order, filter_to_target, test_package, run_in_levels


class TestCommand(Command):
    name = "test"
    help = "Run tests"

    def add_arguments(self, parser: ArgumentParser) -> None:
        parser.add_argument("package", nargs="?", help="Package to test (default: all)")
        parser.add_argument("-j", "--jobs", type=int, default=os.cpu_count(), help="Parallel jobs")

    def run(self, args: Namespace) -> None:
        packages = find_packages(self.root)
        order = build_order(packages)

        if args.package:
            order = filter_to_target(packages, order, args.package)

        build_dir = self.root / "build"

        if not build_dir.exists():
            raise RuntimeError("Build directory not found. Run 'conduit build' first.")

        task = partial(test_package, build_dir=build_dir)
        failed = run_in_levels(packages, order, task, args.jobs, action="Testing")

        if failed:
            raise RuntimeError(f"Tests failed for: {', '.join(failed)}")

        print("All tests passed.")
