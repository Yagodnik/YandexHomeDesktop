import os
import platform
import subprocess
import time
from pathlib import Path

def run_command(command, cwd):
    print(f"\n>>> Running: {' '.join(command)}\n")
    start_time = time.time()

    try:
        process = subprocess.Popen(
            command,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1  # line-buffered
        )

        for line in process.stdout:
            print(line, end='')

        process.wait()
        if process.returncode != 0:
            print(f"\n✖ Command failed with exit code {process.returncode}")
            exit(1)
    except Exception as e:
        print(f"\n✖ Exception while running command: {e}")
        exit(1)

    duration = time.time() - start_time
    print(f"\n✔ Finished in {duration:.2f} seconds")

def main():
    current_os = platform.system()
    print(f"\n💻 Detected OS: {current_os}\n")

    is_mac = current_os == "Darwin"
    is_windows = current_os == "Windows"

    if not is_mac and not is_windows:
        print("✖ Unsupported OS for deployment.")
        exit(1)

    project_root = Path.cwd()
    build_dir = project_root / "build-release"

    if not build_dir.exists():
        print(f"📁 Creating build directory: {build_dir}")
        build_dir.mkdir(parents=True)
    else:
        print(f"📁 Using existing build directory: {build_dir}")

    print("\n=== Step 1: CMake Configuration ===")
    run_command(["cmake", "-DCMAKE_BUILD_TYPE=Release", "..", "-G", "Ninja"], cwd=build_dir)

    print("\n=== Step 2: Building Project ===")
    run_command(["cmake", "--build", ".", "--config", "Release"], cwd=build_dir)

    if is_mac:
        print("\n=== Step 3: Deploying App with macdeployqt ===")
        app_path = build_dir / "YandexHomeDesktop.app"
        qml_path = "../build-release/YandexHomeDesktop"
        run_command([
            "macdeployqt",
            str(app_path),
            f"-qmldir={qml_path}"
        ], cwd=build_dir)
    elif is_windows:
        print("\n=== Step 3: Deploying App with windeployqt ===")
        exe_path = build_dir / "YandexHomeDesktop.exe"
        run_command([
            "windeployqt",
            str(exe_path),
            "--qmldir", str(build_dir / "src/qml/YandexHomeDesktop")
        ], cwd=build_dir)

    print("\n🚀 All steps completed successfully.")

if __name__ == "__main__":
    main()
