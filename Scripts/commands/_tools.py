import os
import shutil
from pathlib import Path

from . import ROOT

SOURCE_DIRS = ("Engine/Source",)

SOURCE_EXTENSIONS = (".cpp", ".cc", ".cxx", ".h", ".hpp", ".hxx", ".inl")

TU_EXTENSIONS = (".cpp", ".cc", ".cxx")

def find_tool(name: str, env_var: str | None = None) -> str | None:
    if env_var:
        override = os.environ.get(env_var)
        if override:
            return shutil.which(override) or override
    return shutil.which(name)

def collect_sources(extensions: tuple[str, ...] = SOURCE_EXTENSIONS) -> list[Path]:
    files: list[Path] = []
    for rel in SOURCE_DIRS:
        directory = ROOT / rel
        if not directory.is_dir():
            continue
        for path in sorted(directory.rglob("*")):
            if path.is_file() and path.suffix in extensions:
                files.append(path)
    return files

def batched(paths: list[Path], max_chars: int = 6000) -> list[list[Path]]:
    batches: list[list[Path]] = []
    current: list[Path] = []
    length = 0
    for path in paths:
        size = len(str(path)) + 1
        if current and length + size > max_chars:
            batches.append(current)
            current, length = [], 0
        current.append(path)
        length += size
    if current:
        batches.append(current)
    return batches
