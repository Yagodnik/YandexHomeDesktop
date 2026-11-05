"""Run Spix flows against the desktop executable and save optional artifacts."""

import argparse
import base64
from contextlib import contextmanager, nullcontext
import os
from pathlib import Path
import shutil
import socket
import subprocess
import tempfile
import time
import xmlrpc.client


WINDOW = "mainWindow"
PAGES = f"{WINDOW}/pages"
HOST = "127.0.0.1"
PORT = 9000


class _TimedTransport(xmlrpc.client.Transport):
    def make_connection(self, host):
        connection = super().make_connection(host)
        connection.timeout = 3
        return connection


def capture(rpc, path):
    png = base64.b64decode(rpc.takeScreenshotAsBase64(WINDOW), validate=True)
    if not png.startswith(b"\x89PNG\r\n\x1a\n"):
        raise RuntimeError("Spix did not return a PNG screenshot")
    path.write_bytes(png)


class SpixSession:
    def __init__(self, app, rpc, artifacts):
        self._app = app
        self.rpc = rpc
        self._artifacts = artifacts
        self._steps = 0

    def wait_for(self, condition, description, *, timeout=5):
        deadline = time.monotonic() + timeout
        while True:
            if self._app.poll() is not None:
                raise RuntimeError(f"Desktop app exited with status {self._app.returncode}")
            try:
                if condition():
                    return
            except ConnectionRefusedError:
                pass
            if time.monotonic() >= deadline:
                raise RuntimeError(f"Timed out waiting for {description}")
            time.sleep(0.1)

    def page_ready(self, item):
        return (self.rpc.existsAndVisible(item)
                and self.rpc.getStringProperty(PAGES, "busy") == "false")

    def in_window(self, item):
        """Check whether an item's bounds are fully inside the app window."""
        x, y, width, height = self.rpc.getBoundingBox(item)
        window_x, window_y, window_width, window_height = self.rpc.getBoundingBox(WINDOW)
        return (window_x <= x < x + width <= window_x + window_width
                and window_y <= y < y + height <= window_y + window_height)

    def checkpoint(self, description):
        self._steps += 1
        print(f"Step {self._steps:02d}: {description}", flush=True)
        if self._artifacts is not None:
            capture(self.rpc, self._artifacts / f"step-{self._steps:02d}.png")


@contextmanager
def desktop_session(executable, arguments=(), *, artifacts=None, capture_ui=False):
    executable = Path(executable).resolve(strict=True)
    with socket.socket() as probe:
        probe.settimeout(1)
        if probe.connect_ex((HOST, PORT)) == 0:
            raise RuntimeError(f"Spix tests need port {PORT} to be free")
    env = dict(
        os.environ,
        QT_QPA_PLATFORM="offscreen",
        QT_QUICK_BACKEND="software",
        QT_QPA_FONTDIR=str(Path(__file__).resolve().parents[2] / "resources" / "fonts"),
    )
    log = (artifacts / "app.log").open("wb") if artifacts is not None else nullcontext()
    with log as output, subprocess.Popen(
        [str(executable), *arguments], env=env,
        stdout=output, stderr=subprocess.STDOUT if output is not None else None,
    ) as app:
        try:
            with xmlrpc.client.ServerProxy(
                f"http://{HOST}:{PORT}", transport=_TimedTransport()
            ) as rpc:
                session = SpixSession(app, rpc, artifacts if capture_ui else None)
                try:
                    session.wait_for(lambda: rpc.getStringProperty(WINDOW, "title"),
                                     "the desktop window", timeout=20)
                    yield session
                    rpc.quit()
                except Exception:
                    if capture_ui and artifacts is not None and app.poll() is None:
                        try:
                            capture(rpc, artifacts / "failure.png")
                        except Exception as error:
                            print(f"Could not capture the failure screenshot: {error}", flush=True)
                    raise
            if app.wait(timeout=10) != 0:
                raise RuntimeError(f"Desktop app exited with status {app.returncode}")
        finally:
            if app.poll() is None:
                app.kill()


def encode_gif(ffmpeg, artifacts, output):
    if not (artifacts / "step-01.png").is_file():
        raise RuntimeError("The flow recorded no checkpoints")
    subprocess.run(
        [ffmpeg, "-nostdin", "-hide_banner", "-loglevel", "error", "-n", "-framerate", "1",
         "-start_number", "1", "-i", str(artifacts / "step-%02d.png"),
         "-filter_complex", "split[a][b];[a]palettegen[p];[b][p]paletteuse",
         "-loop", "0", str(output)],
        check=True, timeout=30,
    )


def save_playback(ffmpeg, artifacts, output, *, required):
    try:
        encode_gif(ffmpeg, artifacts, output)
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        output.unlink(missing_ok=True)
        if required:
            raise
        print(f"Could not encode the GIF: {error}", flush=True)
    else:
        print(f"Recorded {output}", flush=True)


def run_flow_cli(suite, flow, description, *, fixture=False):
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("executable", type=Path, help="desktop executable built with Spix")
    parser.add_argument("--gif", type=Path, help="record a checkpoint GIF using FFmpeg")
    args = parser.parse_args()
    report = os.environ.get("YH_TEST_ARTIFACT_DIR")
    if args.gif is not None and not fixture:
        parser.error("GIF capture is available for fixture flows")
    required = fixture and (os.environ.get("YH_TEST_RECORD_GIF") == "1" or args.gif is not None)
    ffmpeg = shutil.which("ffmpeg") if fixture and (required or report) else None
    if required and ffmpeg is None:
        parser.error("GIF capture requires ffmpeg on PATH")
    if args.gif and (args.gif.suffix.lower() != ".gif" or args.gif.exists() or not args.gif.parent.is_dir()):
        parser.error("--gif must name a new .gif file in an existing directory")

    temporary = tempfile.TemporaryDirectory(prefix="spix-") if ffmpeg and not report else nullcontext()
    with temporary as scratch:
        artifacts = Path(report) / suite if report else Path(scratch) if scratch else None
        if artifacts is not None:
            artifacts.mkdir(exist_ok=True)
            # CTest can repeat a flow within the same report. Keep the last attempt.
            for path in artifacts.glob("step-*.png"):
                path.unlink()
            for name in ("app.log", "failure.png", "playback.gif"):
                (artifacts / name).unlink(missing_ok=True)
        output = args.gif or (artifacts / "playback.gif" if ffmpeg else None)
        try:
            with desktop_session(args.executable, ["--fake-api"] if fixture else [],
                                 artifacts=artifacts, capture_ui=fixture) as session:
                flow(session)
        except Exception:
            if output is not None:
                save_playback(ffmpeg, artifacts, output, required=False)
            raise
        if output is not None:
            save_playback(ffmpeg, artifacts, output, required=required)
