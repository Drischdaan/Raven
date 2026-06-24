import argparse
import subprocess
import sys

from ._tools import batched, collect_sources, find_tool

def run(args: argparse.Namespace) -> int:
    clang_format = find_tool("clang-format", env_var="CLANG_FORMAT")
    if clang_format is None:
        print(
            "error: clang-format not found on PATH. Install LLVM/clang or set "
            "the CLANG_FORMAT environment variable to its path.",
            file=sys.stderr,
        )
        return 1

    files = collect_sources()
    if not files:
        print("No source files found; nothing to format.")
        return 0

    if args.check:
        mode_args = ["--dry-run", "--Werror"]
        verb = "Checking"
    else:
        mode_args = ["-i"]
        verb = "Formatting"

    print(f"{verb} {len(files)} file(s) with clang-format...")

    failed = False
    for batch in batched(files):
        result = subprocess.run([clang_format, *mode_args, *(str(p) for p in batch)])
        if result.returncode != 0:
            failed = True

    if failed:
        if args.check:
            print("Formatting issues found. Run 'nest format' to fix them.", file=sys.stderr)
        else:
            print("clang-format reported errors.", file=sys.stderr)
        return 1

    print("Done." if not args.check else "All files are correctly formatted.")
    return 0

def register(subparsers) -> None:
    parser = subparsers.add_parser("format", help="Run clang-format over the source tree.")
    parser.add_argument("--check", action="store_true", help="Fail if files would be reformatted.")
    parser.set_defaults(func=run)
