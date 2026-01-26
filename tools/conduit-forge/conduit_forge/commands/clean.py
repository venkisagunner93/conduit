import shutil
from argparse import ArgumentParser, Namespace

from .command import Command


class CleanCommand(Command):
    name = "clean"
    help = "Clean build artifacts"

    def add_arguments(self, parser: ArgumentParser) -> None:
        pass

    def run(self, args: Namespace) -> None:
        build_dir = self.root / "build"
        install_dir = self.root / "install"

        if build_dir.exists():
            shutil.rmtree(build_dir)
        if install_dir.exists():
            shutil.rmtree(install_dir)

        print("Cleaned build and install directories.")
