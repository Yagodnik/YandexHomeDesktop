#!/usr/bin/env python3
"""Check the tray popup on a dedicated, interactive desktop."""

import argparse
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile

from spix_harness import WINDOW, desktop_session


SUITE = "TrayPlacement"
SETTINGS = f"{WINDOW}/settingsTab"
TRAY_SWITCH = f"{WINDOW}/traySettingRow/settingSwitch"
NATIVE_HELPER = Path(__file__).with_name("macos_desktop.swift")


def capture_displays(artifacts, checkpoint, count, helper, windows):
    paths = [artifacts / f"step-{checkpoint}-display-{index + 1}.png" for index in range(count)]
    command = [helper, "capture"] if windows else ["/usr/sbin/screencapture", "-x", "-t", "png"]
    subprocess.run([*command, *paths], check=True, timeout=15)
    for path in paths:
        with path.open("rb") as image:
            header = image.read(24)
        if (len(header) != 24 or header[:8] != b"\x89PNG\r\n\x1a\n"
                or min(struct.unpack_from(">II", header, 16)) == 0):
            raise RuntimeError(f"Invalid full-display PNG at {path}")


def tray_bounds(rpc):
    return tuple(map(int, rpc.invokeMethod(WINDOW, "trayIconBounds", []).split(",")))


def contains(outer, inner):
    x, y, width, height = outer
    ix, iy, iw, ih = inner
    return x <= ix and y <= iy and ix + iw <= x + width and iy + ih <= y + height


def check_placement(work_area, anchor, popup, *, below_icon):
    center_x = anchor[0] + anchor[2] / 2
    x, y, width, height = popup
    left, top, usable_width, usable_height = work_area
    if width <= 0 or height <= 0 or not contains(work_area, popup):
        raise RuntimeError(f"Popup {popup} is outside the tray display work area {work_area}")

    # Center on the icon, on the usable side of the menu bar or taskbar.
    expected_x = min(max(center_x - width / 2, left), left + usable_width - width)
    aligned_y = anchor[1] + anchor[3] + 5 if below_icon else anchor[1] - height - 6
    expected_y = min(max(aligned_y, top), top + usable_height - height)
    if abs(x - expected_x) > 2 or abs(y - expected_y) > 2:
        raise RuntimeError(f"Popup {popup} is misplaced for tray icon {anchor} in {work_area}. "
                           f"Expected origin near ({expected_x}, {expected_y})")


def locate_anchor(displays, anchor, windows):
    center_x = anchor[0] + anchor[2] / 2
    center_y = anchor[1] + anchor[3] / 2
    if min(anchor[2:]) > 0:
        for display in displays:
            x, y, width, height = display["bounds"]
            scale = display["scale"] if windows else 1
            if x <= center_x < x + width * scale and y <= center_y < y + height * scale:
                logical_anchor = (x + (anchor[0] - x) / scale, y + (anchor[1] - y) / scale,
                                  anchor[2] / scale, anchor[3] / scale)
                return display["work_area"], logical_anchor
    raise RuntimeError(f"Tray icon {anchor} is outside the configured displays or hidden")


def run_flow(session, displays, artifacts, helper, windows):
    rpc = session.rpc
    session.wait_for(lambda: session.page_ready(SETTINGS), "the Settings tab", timeout=20)
    rpc.mouseClick(SETTINGS)
    session.wait_for(lambda: session.page_ready(TRAY_SWITCH) and session.in_window(TRAY_SWITCH),
                     "the tray mode switch")
    rpc.mouseClick(TRAY_SWITCH)
    icon_description = "the pinned Windows notification icon" if windows else "the macOS menu bar icon"
    session.wait_for(lambda: rpc.getStringProperty(WINDOW, "trayModeEnabled") == "true"
                     and min(tray_bounds(rpc)[2:]) > 0, icon_description, timeout=10)

    rpc.invokeMethod(WINDOW, "hide", [])
    session.wait_for(lambda: rpc.getStringProperty(WINDOW, "visible") == "false",
                     "the popup to be hidden before native activation")
    anchor = tray_bounds(rpc)
    work_area, logical_anchor = locate_anchor(displays, anchor, windows)
    if windows and logical_anchor[1] + logical_anchor[3] / 2 < work_area[1] + work_area[3]:
        raise RuntimeError(f"Expected the pinned tray icon in the bottom taskbar, got {logical_anchor}")
    center = (anchor[0] + anchor[2] / 2, anchor[1] + anchor[3] / 2)
    if windows:
        center = tuple(map(int, center))
    subprocess.run([helper, "click", *map(str, center)], check=True, timeout=5)
    session.wait_for(lambda: rpc.getStringProperty(WINDOW, "active") == "true"
                     and rpc.getStringProperty(WINDOW, "visible") == "true", "the tray popup")

    popup = rpc.getBoundingBox(WINDOW)
    check_placement(work_area, logical_anchor, popup, below_icon=not windows)
    capture_displays(artifacts, "01", len(displays), helper, windows)
    print(f"Tray icon {anchor}, popup {popup}, full-display PNGs in {artifacts}", flush=True)


def run_test(executable, expected, helper, platform):
    displays = json.loads(subprocess.check_output([helper, "displays"], timeout=5))
    layout = sorted([*display["bounds"], display["scale"]] for display in displays)
    if not layout or layout != expected:
        raise RuntimeError(f"Expected display layout {expected}, observed {layout}")
    profile = os.environ.get("YH_TRAY_DISPLAY_PROFILE")
    windows = platform == "windows"
    if profile == "tray-compact" and len(layout) != 1:
        raise RuntimeError("The compact tray profile requires one display")
    if profile == "tray-mixed":
        if len(layout) != 2 or layout[0][4] == layout[1][4] or not any(
            x < 0 or y < 0 for x, y, *_ in layout
        ):
            raise RuntimeError("The mixed tray profile requires two differently scaled displays "
                               "with the secondary left of or above the main display")
        if windows and not any(x == y == 0 and scale > 1 for x, y, _, _, scale in layout):
            raise RuntimeError("The Windows mixed tray profile requires a scaled main display")
    print(f"Native displays: {displays}", flush=True)

    artifacts = Path(os.environ.get("YH_TEST_ARTIFACT_DIR", ".")) / SUITE
    artifacts.mkdir(parents=True, exist_ok=True)
    for path in artifacts.glob("step-*-display-*.png"):
        path.unlink()
    with desktop_session(executable, ("--fake-api",), artifacts=artifacts, platform=platform) as session:
        try:
            run_flow(session, displays, artifacts, helper, windows)
        except Exception:
            try:
                capture_displays(artifacts, "failure", len(displays), helper, windows)
            except (OSError, RuntimeError, subprocess.SubprocessError) as error:
                print(f"Could not capture the full desktop on failure: {error}", flush=True)
            raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("executable", type=Path, help="Debug desktop executable built with Spix")
    parser.add_argument("--windows-helper", type=Path, help="path to the Windows desktop helper")
    args = parser.parse_args()
    try:
        expected = sorted(json.loads(os.environ["YH_TRAY_DISPLAY_LAYOUT"]))
    except (KeyError, ValueError, TypeError) as error:
        parser.error(f"Set YH_TRAY_DISPLAY_LAYOUT to the expected JSON layout. {error}")
    if sys.platform == "win32":
        if args.windows_helper is None:
            parser.error("--windows-helper is required on Windows")
        run_test(args.executable, expected, args.windows_helper, "windows")
        return
    if sys.platform != "darwin":
        parser.error("Native tray testing is available on Windows and macOS")
    with tempfile.TemporaryDirectory(prefix="yh-tray-") as scratch:
        helper = Path(scratch) / "macos_desktop"
        subprocess.run(["/usr/bin/xcrun", "swiftc", "-module-cache-path", scratch,
                        "-o", helper, NATIVE_HELPER], check=True, timeout=30)
        run_test(args.executable, expected, helper, "cocoa")


if __name__ == "__main__":
    main()
