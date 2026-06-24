import argparse
import os
import subprocess
import sys
from pathlib import Path

from . import ROOT
from ._tools import TU_EXTENSIONS, batched, collect_sources, find_tool

BUILD_DIR_CANDIDATES = (
    "Build/Tooling",
    "Build",
    "build",
    "out/build",
    "cmake-build-debug",
    "cmake-build-release",
)

def find_compile_db(explicit: str | None) -> Path | None:
    candidates: list[Path] = []
    if explicit:
        candidates.append(Path(explicit))
    env = os.environ.get("RAVEN_BUILD_DIR")
    if env:
        candidates.append(Path(env))
    candidates.extend(ROOT / rel for rel in BUILD_DIR_CANDIDATES)

    for directory in candidates:
        directory = directory if directory.is_absolute() else (ROOT / directory)
        if (directory / "compile_commands.json").is_file():
            return directory
    return None

def run(args: argparse.Namespace) -> int:
    clang_tidy = find_tool("clang-tidy", env_var="CLANG_TIDY")
    if clang_tidy is None:
        print(
            "error: clang-tidy not found on PATH. Install LLVM/clang or set "
            "the CLANG_TIDY environment variable to its path.",
            file=sys.stderr,
        )
        return 1

    build_dir = find_compile_db(args.build_dir)
    if build_dir is None:
        print(
            "error: compile_commands.json not found. Configure the project with "
            "CMake (-DCMAKE_EXPORT_COMPILE_COMMANDS=ON) or pass --build-dir.",
            file=sys.stderr,
        )
        return 1

    files = collect_sources(TU_EXTENSIONS)
    if not files:
        print("No source files found; nothing to lint.")
        return 0

    tidy_args = ["-p", str(build_dir), "--quiet"]
    if args.fix:
        tidy_args.append("--fix")

    print(f"Linting {len(files)} file(s) with clang-tidy (compile db: {build_dir})...")

    failed = False
    for batch in batched(files):
        result = subprocess.run([clang_tidy, *tidy_args, *(str(p) for p in batch)])
        if result.returncode != 0:
            failed = True

    if failed:
        print("clang-tidy reported issues.", file=sys.stderr)
        return 1

    print("No issues found.")
    return 0

def register(subparsers) -> None:
    parser = subparsers.add_parser("lint", help="Run clang-tidy over the source tree.")
    parser.add_argument(
        "--build-dir",
        help="Directory containing compile_commands.json (defaults to autodetect).",
    )
    parser.add_argument("--fix", action="store_true", help="Apply clang-tidy's suggested fixes.")
    parser.set_defaults(func=run)
