from abc import ABC, abstractmethod
from argparse import ArgumentParser, Namespace
from pathlib import Path


class Command(ABC):
    """Base class for CLI commands."""

    name: str = ""
    help: str = ""

    def __init__(self, root: Path):
        self.root = root

    @abstractmethod
    def add_arguments(self, parser: ArgumentParser) -> None:
        """Add command-specific arguments to the parser."""
        pass

    @abstractmethod
    def run(self, args: Namespace) -> None:
        """Execute the command."""
        pass
