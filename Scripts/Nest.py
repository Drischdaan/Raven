#!/usr/bin/env python3

import argparse
import sys

from commands import COMMANDS

def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="nest",
        description="Raven's development tooling CLI.",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)
    for command in COMMANDS:
        command.register(subparsers)
    return parser

def main(argv: list[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    return args.func(args)

if __name__ == "__main__":
    sys.exit(main())
