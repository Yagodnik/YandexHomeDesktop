"""Cross-platform commands behind the justfile; requires only Python's stdlib."""

import os
from pathlib import Path
import subprocess
import sys


def main(command, args):
    config = os.environ.get("YH_JUST_CONFIG", "Debug")
    directory = Path(os.environ.get("YH_JUST_BUILD_DIR", "build-just/debug"))
    desktop = os.environ.get("YH_JUST_DESKTOP", "OFF" if sys.platform.startswith("linux") else "ON")
    build_command = ["cmake", "--build", str(directory), "--config", config]

    if command == "help":
        return subprocess.call(["just", "--list"])
    if command == "configure":
        pch = os.environ.get("YH_JUST_PCH", "ON")
        if config not in ("Debug", "Release", "RelWithDebInfo", "MinSizeRel"):
            print("config must be Debug, Release, RelWithDebInfo, or MinSizeRel", file=sys.stderr)
            return 2
        if desktop not in ("ON", "OFF") or pch not in ("ON", "OFF"):
            print("desktop and pch must be ON or OFF", file=sys.stderr)
            return 2
        invocation = ["cmake", "-S", ".", "-B", str(directory),
                      "-G", os.environ.get("YH_JUST_GENERATOR", "Ninja"),
                      f"-DCMAKE_BUILD_TYPE={config}", "-DBUILD_TESTING=ON",
                      f"-DBUILD_DESKTOP_APP={desktop}", f"-DYH_ENABLE_PCH={pch}"]
        prefix = os.environ.get("YH_JUST_QT_PREFIX", "")
        if prefix:
            invocation.append(f"-DCMAKE_PREFIX_PATH={prefix}")
        if desktop == "ON":
            auth_config = Path(os.environ.get("YH_JUST_AUTH_CONFIG", "resources/auth/example.json")).resolve()
            invocation.append(f"-DYH_AUTH_CONFIG_FILE={auth_config}")
        return subprocess.call(invocation + args)
    if command == "build":
        if not (directory / "CMakeCache.txt").is_file():
            result = subprocess.call(["just", "configure"])
            if result:
                return result
        return subprocess.call(build_command + ["--parallel", os.environ.get("YH_JUST_JOBS", "4"), *args])
    if command == "test":
        return subprocess.call(["ctest", "--test-dir", str(directory), "-C", config,
                                "--output-on-failure", "--parallel", os.environ.get("YH_JUST_JOBS", "4"), *args])
    if command == "demo":
        if config != "Debug" or desktop != "ON":
            print("Fixture mode requires config=Debug and desktop=ON", file=sys.stderr)
            return 2
        return subprocess.call(["just", "run", "--fake-api", *args])
    if command in ("run", "cli"):
        if command == "run":
            relative = (Path("YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop")
                        if sys.platform == "darwin" else Path("YandexHomeDesktop.exe"))
        else:
            relative = Path("YandexHomeCli.exe" if sys.platform == "win32" else "YandexHomeCli")
        candidates = [directory / config / relative, directory / relative]
        executable = next((path.resolve() for path in candidates if path.is_file()), None)
        if executable is None:
            print("Executable not found; configure with desktop=ON on Windows or macOS", file=sys.stderr)
            return 2
        return subprocess.call([str(executable), *args])
    if command == "install":
        return subprocess.call(["cmake", "--install", str(directory), "--config", config,
                                "--prefix", str(directory.resolve() / "stage"), *args])
    if command == "clean":
        return subprocess.call(build_command + ["--target", "clean"])
    if command == "translations-check":
        for script in (["scripts/update-data-translation-markers.py", "--check"],
                       ["scripts/check-translations.py"]):
            result = subprocess.call([sys.executable, *script])
            if result:
                return result
        return 0
    if command == "translations-update":
        if desktop != "ON":
            print("translations-update requires a desktop profile", file=sys.stderr)
            return 2
        result = subprocess.call([sys.executable, "scripts/update-data-translation-markers.py"])
        if result:
            return result
        if not (directory / "CMakeCache.txt").is_file():
            result = subprocess.call(["just", "configure"])
            if result:
                return result
        return subprocess.call(build_command + ["--target", "update_translations"])
    print(f"Unknown development command: {command}", file=sys.stderr)
    return 2


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit("Usage: dev.py <command> [arguments...]; use just --list for commands")
    sys.exit(main(sys.argv[1], sys.argv[2:]))
