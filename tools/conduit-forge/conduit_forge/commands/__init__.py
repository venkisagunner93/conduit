from .build import BuildCommand
from .test import TestCommand
from .list import ListCommand
from .clean import CleanCommand

ALL_COMMANDS = [
    BuildCommand,
    TestCommand,
    ListCommand,
    CleanCommand,
]
