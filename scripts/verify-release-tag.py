"""Reject release tags that do not describe the checked-out main commit."""

import re
import subprocess
import sys
from pathlib import Path


def git(*args: str) -> str:
    return subprocess.check_output(["git", *args], text=True).strip()


def main() -> None:
    if len(sys.argv) != 2 or not re.fullmatch(r"v\d+\.\d+\.\d+", sys.argv[1]):
        raise SystemExit("Release tag must have the form vMAJOR.MINOR.PATCH")

    tag = sys.argv[1]
    cmake = Path("CMakeLists.txt").read_text(encoding="utf-8")
    match = re.search(r"project\(YandexHomeDesktop\s+VERSION\s+(\d+\.\d+\.\d+)", cmake)
    if not match or tag[1:] != match.group(1):
        raise SystemExit("Release tag does not match the CMake project version")

    try:
        if git("rev-list", "-n", "1", f"refs/tags/{tag}") != git("rev-parse", "HEAD"):
            raise SystemExit("Release tag does not point to the checked-out commit")
        subprocess.run(
            ["git", "merge-base", "--is-ancestor", "HEAD", "origin/main"],
            check=True,
        )
    except subprocess.CalledProcessError as exc:
        raise SystemExit("Release tag must point to a commit on main") from exc

    print(f"Verified release {tag}")


if __name__ == "__main__":
    main()
