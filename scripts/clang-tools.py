#!/usr/bin/env python3
"""Format or analyze C++ files, defaulting to application startup code."""

import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys


ROOT = Path(__file__).resolve().parents[1]
CPP_EXTENSIONS = {".cpp", ".cc", ".cxx", ".h", ".hh", ".hpp", ".mm"}


def find_tool(name):
    versions = dict(line.split("==") for line in
                    (ROOT / "requirements-clang-tools.txt").read_text(encoding="utf-8").splitlines()
                    if line and not line.startswith("#"))
    local = ROOT / "build-clang-tools" / ("Scripts" if sys.platform == "win32" else "bin")
    local = local / (name + ".exe" if sys.platform == "win32" else name)
    tool = str(local) if local.is_file() else shutil.which(name)
    if not tool:
        raise ValueError(f"Install the pinned tools from requirements-clang-tools.txt; missing {name}.")
    version = subprocess.check_output([tool, "--version"], text=True)
    if f"version {versions[name]}" not in version:
        raise ValueError(f"{name} {versions[name]} is required; found {version.strip()}.")
    return tool


def compiler_header_args(cache):
    compiler = next(line.split("=", 1)[1] for line in cache.splitlines()
                    if line.startswith("CMAKE_CXX_COMPILER:"))
    probe = subprocess.run([compiler, "-E", "-x", "c++", "-v", "-"], input="",
                           text=True, capture_output=True, check=True)
    args = []
    searching = False
    for line in probe.stderr.splitlines():
        if "#include <...> search starts here:" in line:
            searching = True
        elif "End of search list." in line:
            searching = False
        elif searching:
            framework = " (framework directory)"
            path = line.strip()
            flag = "-iframework" if path.endswith(framework) else "-isystem"
            path = path.removesuffix(framework)
            args.extend([f"--extra-arg={flag}", f"--extra-arg={path}"])
    if sys.platform == "win32":
        target = subprocess.check_output([compiler, "-dumpmachine"], text=True).strip()
        args.append(f"--extra-arg=--target={target}")
    return args


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=("format", "check", "tidy"))
    parser.add_argument("--build-dir", type=Path)
    parser.add_argument("paths", nargs="*", type=Path, default=[])
    args = parser.parse_args()

    files = set()
    for path in args.paths or [ROOT / "src/app"]:
        path = path.resolve()
        if not path.exists():
            raise ValueError(f"Path does not exist: {path}")
        candidates = path.rglob("*") if path.is_dir() else [path]
        files.update(file.resolve() for file in candidates
                     if file.is_file() and file.suffix in CPP_EXTENSIONS)
    if not files:
        raise ValueError("No C++ files matched the supplied paths.")

    if args.mode != "tidy":
        tool = find_tool("clang-format")
        flags = ["-i"] if args.mode == "format" else ["--dry-run", "--Werror"]
        ordered = sorted(files)
        for offset in range(0, len(ordered), 50):
            subprocess.run([tool, "--style=file", *flags,
                            *(str(file) for file in ordered[offset:offset + 50])],
                           cwd=ROOT, check=True)
        print(f"{'Formatted' if args.mode == 'format' else 'Checked'} {len(files)} C++ files.")
        return

    if args.build_dir is None:
        raise ValueError("tidy requires --build-dir pointing to a configured build.")
    build = args.build_dir.resolve()
    database = build / "compile_commands.json"
    if not database.is_file():
        raise ValueError("Missing compile_commands.json. Configure with Ninja and build first.")
    cache = (build / "CMakeCache.txt").read_text(encoding="utf-8")
    if "YH_ENABLE_PCH:BOOL=ON" in cache:
        raise ValueError("Configure this profile with -DYH_ENABLE_PCH=OFF before running tidy "
                         "so clang-tidy can parse headers without compiler-specific PCH files.")
    commands = json.loads(database.read_text(encoding="utf-8"))
    compiled = {Path(entry["directory"]).joinpath(entry["file"]).resolve() for entry in commands}
    files &= compiled
    if not files:
        raise ValueError("No selected source files are compiled in this build profile.")
    tool = find_tool("clang-tidy")
    header_args = compiler_header_args(cache)
    for file in sorted(files):
        print(f"Checking {file.relative_to(ROOT)}", flush=True)
        subprocess.run([tool, "-p", str(build), *header_args, str(file)], cwd=ROOT, check=True)
    print(f"Analyzed {len(files)} C++ source files.")


if __name__ == "__main__":
    try:
        main()
    except (ValueError, subprocess.CalledProcessError) as error:
        print(error, file=sys.stderr)
        raise SystemExit(1)
