import argparse
from pathlib import Path

from .commands import ALL_COMMANDS


def main():
    parser = argparse.ArgumentParser(prog="forge", description="Conduit build tool")
    subparsers = parser.add_subparsers(dest="command", required=True)

    root = Path.cwd()

    # Register all commands
    commands = {}
    for command_class in ALL_COMMANDS:
        cmd = command_class(root)
        cmd_parser = subparsers.add_parser(cmd.name, help=cmd.help)
        cmd.add_arguments(cmd_parser)
        commands[cmd.name] = cmd

    args = parser.parse_args()

    try:
        commands[args.command].run(args)
    except RuntimeError as e:
        print(f"Error: {e}")
        exit(1)


if __name__ == "__main__":
    main()
