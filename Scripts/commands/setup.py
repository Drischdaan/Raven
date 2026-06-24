import argparse
import os
import platform
import shutil
import subprocess
import sys
from pathlib import Path

from . import ROOT
from ._tools import find_tool

HOOKS_DIR = ".githooks"

PLATFORM_GENERATORS = {
    "Windows": "Visual Studio 18 2026",
    "Darwin": "Ninja",
    "Linux": "Ninja",
}

DEFAULT_GENERATOR = "Ninja"

PLATFORM_TOOLCHAINS = {
    "Windows": "Engine/Build/toolchains/msvc-windows.cmake",
}

BUILD_DIR = "Build"
TOOLING_DIR = "Build/Tooling"

COMPILE_DB_GENERATORS = (
    "Ninja",
    "Ninja Multi-Config",
    "Unix Makefiles",
    "MSYS Makefiles",
    "MinGW Makefiles",
    "NMake Makefiles",
    "NMake Makefiles JOM",
)

def default_generator() -> str:
    return PLATFORM_GENERATORS.get(platform.system(), DEFAULT_GENERATOR)

def is_visual_studio(generator: str) -> bool:
    return generator.startswith("Visual Studio")

def resolve_toolchain(explicit: str | None, generator: str) -> str | None:
    chosen = explicit or os.environ.get("RAVEN_TOOLCHAIN")

    if chosen is None and not is_visual_studio(generator):
        chosen = PLATFORM_TOOLCHAINS.get(platform.system())

    if chosen is None:
        return None

    path = Path(chosen)
    if not path.is_absolute():
        path = ROOT / path
    if not path.is_file():
        print(f"warning: toolchain file '{path}' not found; using the default compiler.", file=sys.stderr)
        return None

    return str(path)

def needs_msvc_environment(toolchain: str | None) -> bool:
    if platform.system() != "Windows" or toolchain is None:
        return False
    msvc = PLATFORM_TOOLCHAINS.get("Windows")
    if msvc is None:
        return False
    return Path(toolchain).resolve() == (ROOT / msvc).resolve()

def msvc_on_path() -> bool:
    return bool(shutil.which("cl") and shutil.which("rc"))

def _vswhere() -> Path | None:
    program_files = (
        os.environ.get("ProgramFiles(x86)")
        or os.environ.get("ProgramFiles")
        or r"C:\Program Files (x86)"
    )
    candidate = Path(program_files) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    return candidate if candidate.is_file() else None

def msvc_environment(arch: str = "x64") -> dict[str, str] | None:
    vswhere = _vswhere()
    if vswhere is None:
        return None

    query = subprocess.run(
        [
            str(vswhere),
            "-latest", "-products", "*",
            "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "-property", "installationPath",
        ],
        capture_output=True, text=True,
    )
    install_paths = query.stdout.splitlines()
    if not install_paths:
        return None

    vcvarsall = Path(install_paths[0].strip()) / "VC" / "Auxiliary" / "Build" / "vcvarsall.bat"
    if not vcvarsall.is_file():
        return None

    try:
        output = subprocess.check_output(
            f'cmd /u /c "{vcvarsall}" {arch} && set',
            stderr=subprocess.DEVNULL,
        ).decode("utf-16le", errors="replace")
    except subprocess.CalledProcessError:
        return None

    env: dict[str, str] = {}
    for line in output.splitlines():
        key, sep, value = line.partition("=")
        if sep and key:
            env[key] = value
    return env or None

def install_hooks() -> int:
    hooks_path = ROOT / HOOKS_DIR
    if not hooks_path.is_dir():
        print(f"error: hooks directory '{HOOKS_DIR}' not found.", file=sys.stderr)
        return 1

    result = subprocess.run(
        ["git", "config", "core.hooksPath", HOOKS_DIR],
        cwd=ROOT,
    )
    if result.returncode != 0:
        print("error: failed to configure git hooks.", file=sys.stderr)
        return 1

    print(f"Git hooks installed (core.hooksPath -> {HOOKS_DIR}).")
    return 0

def configure(cmake: str, generator: str, build_dir: str, toolchain: str | None = None) -> int:
    suffix = f" (toolchain: {Path(toolchain).name})" if toolchain else ""
    print(f"Generating project files with '{generator}' in '{build_dir}'{suffix}...")

    command = [
        cmake,
        "-S", str(ROOT),
        "-B", str(ROOT / build_dir),
        "-G", generator,
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
    ]
    if toolchain:
        command.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")

    env = None
    if needs_msvc_environment(toolchain) and not msvc_on_path():
        env = msvc_environment()
        if env is None:
            print(
                "warning: could not locate the MSVC developer environment (vswhere/vcvarsall). "
                "Run from a Developer Command Prompt, or install the Visual C++ build tools.",
                file=sys.stderr,
            )

    result = subprocess.run(command, env=env)
    if result.returncode != 0:
        print(f"error: CMake configuration failed for generator '{generator}'.", file=sys.stderr)
    return result.returncode

def run(args: argparse.Namespace) -> int:
    print("Setting up environment...")

    if install_hooks() != 0:
        return 1

    if args.skip_project_files:
        return 0

    cmake = find_tool("cmake", env_var="CMAKE")
    if cmake is None:
        print(
            "error: cmake not found on PATH. Install CMake (>=4.2.3) or set the "
            "CMAKE environment variable to its path.",
            file=sys.stderr,
        )
        return 1

    generator = args.generator or os.environ.get("RAVEN_GENERATOR") or default_generator()

    if configure(cmake, generator, BUILD_DIR, resolve_toolchain(args.toolchain, generator)) != 0:
        return 1

    if generator not in COMPILE_DB_GENERATORS:
        if find_tool("ninja") is None:
            print(
                "warning: ninja not found; skipping the tooling compile database. "
                "clang-tidy/lint will not work until a Ninja or Makefile build is configured.",
                file=sys.stderr,
            )
        else:
            print("Primary generator has no compile database; configuring a Ninja tree for tooling...")
            if configure(cmake, "Ninja", TOOLING_DIR, resolve_toolchain(args.toolchain, "Ninja")) != 0:
                return 1

    print("Setup complete.")
    return 0

def register(subparsers) -> None:
    parser = subparsers.add_parser("setup", help="Set up the development environment.")
    parser.add_argument(
        "--generator",
        help="CMake generator to use (defaults to the platform default).",
    )
    parser.add_argument(
        "--toolchain",
        help="CMake toolchain file to use (defaults to the platform default).",
    )
    parser.add_argument(
        "--skip-project-files",
        action="store_true",
        help="Only install git hooks; don't generate project files.",
    )
    parser.set_defaults(func=run)
