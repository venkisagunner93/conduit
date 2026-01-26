from argparse import ArgumentParser, Namespace

from .command import Command
from ..utils import find_packages, build_order


class ListCommand(Command):
    name = "list"
    help = "List packages"

    def add_arguments(self, parser: ArgumentParser) -> None:
        parser.add_argument("--order", action="store_true", help="Show build order")

    def run(self, args: Namespace) -> None:
        packages = find_packages(self.root)

        if args.order:
            for name in build_order(packages):
                print(name)
        else:
            for name, pkg in packages.items():
                deps = ", ".join(pkg.dependencies) or "none"
                print(f"{name}: depends on [{deps}]")
