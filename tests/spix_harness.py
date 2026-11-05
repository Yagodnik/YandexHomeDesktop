"""Shared runner for standalone Spix tests against the desktop executable."""

import argparse
import base64
from contextlib import contextmanager
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
FPS = 8


class _TimedTransport(xmlrpc.client.Transport):
    def make_connection(self, host):
        connection = super().make_connection(host)
        connection.timeout = 3
        return connection


class _GifFrames:
    def __init__(self, directory):
        self.directory = directory
        self.count = 0

    def capture(self, rpc, seconds):
        start = time.monotonic()
        for index in range(round(FPS * seconds)):
            time.sleep(max(0, start + index / FPS - time.monotonic()))
            png = base64.b64decode(rpc.takeScreenshotAsBase64(WINDOW), validate=True)
            if not png.startswith(b"\x89PNG\r\n\x1a\n"):
                raise RuntimeError("Spix did not return a PNG screenshot")
            (self.directory / f"{self.count:04d}.png").write_bytes(png)
            self.count += 1

    def encode(self, ffmpeg, output):
        if not self.count:
            raise RuntimeError("The flow recorded no frames. Add a checkpoint to capture the window.")
        subprocess.run(
            [ffmpeg, "-nostdin", "-hide_banner", "-loglevel", "error", "-n",
             "-framerate", str(FPS), "-i", str(self.directory / "%04d.png"),
             "-filter_complex", "split[a][b];[a]palettegen[p];[b][p]paletteuse",
             "-loop", "0", str(output)],
            check=True,
        )


class SpixSession:
    def __init__(self, app, rpc, recording):
        self._app = app
        self.rpc = rpc
        self._recording = recording

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

    def checkpoint(self, description=None, *, seconds=0.75):
        """Optionally print progress and sample the window for a GIF."""
        if description:
            print(description, flush=True)
        if self._recording is not None:
            self._recording.capture(self.rpc, seconds)


@contextmanager
def desktop_session(executable, arguments=(), *, recording=None):
    executable = Path(executable).resolve(strict=True)
    with socket.socket() as probe:
        probe.settimeout(1)
        if probe.connect_ex((HOST, PORT)) == 0:
            raise RuntimeError(f"Spix tests need port {PORT} to be free")
    env = dict(
        os.environ,
        QT_QPA_PLATFORM="offscreen",
        QT_QUICK_BACKEND="software",
        QT_QPA_FONTDIR=str(Path(__file__).resolve().parents[1] / "resources" / "fonts"),
    )
    with subprocess.Popen([str(executable), *arguments], env=env) as app:
        try:
            with xmlrpc.client.ServerProxy(
                f"http://{HOST}:{PORT}", transport=_TimedTransport()
            ) as rpc:
                session = SpixSession(app, rpc, recording)
                session.wait_for(lambda: rpc.getStringProperty(WINDOW, "title"),
                                 "the desktop window", timeout=20)
                yield session
                rpc.quit()

            if app.wait(timeout=10) != 0:
                raise RuntimeError(f"Desktop app exited with status {app.returncode}")
        finally:
            if app.poll() is None:
                app.kill()


def run_fixture_cli(flow, description):
    """Run a flow(session) callback in a fresh Debug app with bundled fixture data."""
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("executable", type=Path, help="Debug desktop executable built with Spix")
    parser.add_argument("--gif", type=Path, help="record to a new .gif file using FFmpeg")
    args = parser.parse_args()

    if args.gif is None:
        with desktop_session(args.executable, ["--fake-api"]) as session:
            flow(session)
        return

    if args.gif.suffix.lower() != ".gif" or args.gif.exists() or not args.gif.parent.is_dir():
        parser.error("--gif must name a new .gif file in an existing directory")
    ffmpeg = shutil.which("ffmpeg")
    if ffmpeg is None:
        parser.error("--gif requires ffmpeg on PATH")
    with tempfile.TemporaryDirectory(prefix="spix-frames-") as directory:
        frames = _GifFrames(Path(directory))
        with desktop_session(args.executable, ["--fake-api"], recording=frames) as session:
            flow(session)
        frames.encode(ffmpeg, args.gif)
    print(f"Recorded {args.gif}")
