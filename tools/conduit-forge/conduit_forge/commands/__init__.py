from .build import BuildCommand
from .test import TestCommand
from .list import ListCommand
from .clean import CleanCommand
from .pkg import PkgCommand

ALL_COMMANDS = [
    BuildCommand,
    TestCommand,
    ListCommand,
    CleanCommand,
    PkgCommand,
]
