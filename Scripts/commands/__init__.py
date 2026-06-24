from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

from . import setup, format, lint

COMMANDS = (setup, format, lint)
