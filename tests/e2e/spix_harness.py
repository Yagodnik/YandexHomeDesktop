"""Run Spix flows against the desktop executable and save optional artifacts."""

import argparse
import base64
from contextlib import contextmanager, nullcontext
import os
from pathlib import Path
import shutil
import socket
import struct
import subprocess
import tempfile
import threading
import time
import xmlrpc.client


WINDOW = "mainWindow"
PAGES = f"{WINDOW}/pages"
HOST = "127.0.0.1"
PORT = 9000
RECORD_FPS = 20


class _TimedTransport(xmlrpc.client.Transport):
    def make_connection(self, host):
        connection = super().make_connection(host)
        connection.timeout = 3
        return connection


def connect():
    return xmlrpc.client.ServerProxy(f"http://{HOST}:{PORT}", transport=_TimedTransport())


def capture_frame(rpc):
    png = base64.b64decode(rpc.takeScreenshotAsBase64(WINDOW), validate=True)
    if not png.startswith(b"\x89PNG\r\n\x1a\n"):
        raise RuntimeError("Spix did not return a PNG screenshot")
    return png


def capture_checkpoint(rpc, path):
    """Wait for a quiet frame, with a cap for spinners and other looping visuals."""
    frame = capture_frame(rpc)
    unchanged_since = time.monotonic()
    deadline = unchanged_since + 1.0
    while time.monotonic() < deadline:
        time.sleep(0.05)
        latest = capture_frame(rpc)
        now = time.monotonic()
        if latest != frame:
            frame = latest
            unchanged_since = now
        elif now - unchanged_since >= 0.2:
            break
    path.write_bytes(frame)


class FrameRecorder:
    def __init__(self, directory):
        self.directory = directory
        self.size = (0, 0)
        self.frame_count = 0
        self.error = None
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._record)

    def __enter__(self):
        self._thread.start()
        return self

    def __exit__(self, *_):
        self._stop.set()
        self._thread.join()

    def _record(self):
        interval = 1 / RECORD_FPS
        try:
            with connect() as rpc:
                while not self._stop.is_set():
                    start = time.monotonic()
                    frame = capture_frame(rpc)
                    width, height = struct.unpack_from(">II", frame, 16)
                    self.size = (max(self.size[0], width), max(self.size[1], height))
                    self.frame_count += 1
                    (self.directory / f"frame-{self.frame_count:06d}.png").write_bytes(frame)
                    self._stop.wait(max(0, interval - (time.monotonic() - start)))
        except (OSError, ValueError, RuntimeError, struct.error, xmlrpc.client.Error) as error:
            self.error = error


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

    def click_burst(self, *items):
        """Queue consecutive clicks in one RPC request without waits between clicks."""
        batch = xmlrpc.client.MultiCall(self.rpc)
        for item in items:
            batch.mouseClick(item)
        # Spix posts the mouse events to Qt. Yield one event-loop turn before queries.
        batch.wait(0)
        list(batch())

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
            capture_checkpoint(self.rpc, self._artifacts / f"step-{self._steps:02d}.png")


@contextmanager
def desktop_session(executable, arguments=(), *, artifacts=None, capture_ui=False, recorder=None,
                    platform="offscreen"):
    executable = Path(executable).resolve(strict=True)
    with socket.socket() as probe:
        probe.settimeout(1)
        if probe.connect_ex((HOST, PORT)) == 0:
            raise RuntimeError(f"Spix tests need port {PORT} to be free")
    env = dict(os.environ, QT_QPA_PLATFORM=platform)
    if platform == "offscreen":
        env.update(
            QT_QUICK_BACKEND="software",
            QT_QPA_FONTDIR=str(Path(__file__).resolve().parents[2] / "resources" / "fonts"),
        )
    log = (artifacts / "app.log").open("wb") if artifacts is not None else nullcontext()
    with log as output, subprocess.Popen(
        [str(executable), *arguments], env=env,
        stdout=output, stderr=subprocess.STDOUT if output is not None else None,
    ) as app:
        try:
            with connect() as rpc:
                session = SpixSession(app, rpc, artifacts if capture_ui else None)
                try:
                    session.wait_for(lambda: rpc.getStringProperty(WINDOW, "title"),
                                     "the desktop window", timeout=20)
                    with recorder if recorder is not None else nullcontext():
                        yield session
                    rpc.quit()
                except Exception:
                    if capture_ui and artifacts is not None and app.poll() is None:
                        try:
                            (artifacts / "failure.png").write_bytes(capture_frame(rpc))
                        except Exception as error:
                            print(f"Could not capture the failure screenshot: {error}", flush=True)
                    if app.poll() is None:
                        # Release the shared listener before CTest starts the next flow.
                        try:
                            rpc.quit()
                            app.wait(timeout=3)
                        except (OSError, xmlrpc.client.Error, subprocess.TimeoutExpired):
                            pass
                    raise
            if app.wait(timeout=10) != 0:
                raise RuntimeError(f"Desktop app exited with status {app.returncode}")
        finally:
            if app.poll() is None:
                app.kill()


def encode_gif(ffmpeg, recorder, output):
    if not recorder.frame_count:
        raise RuntimeError("The flow recorded no frames")
    width, height = recorder.size
    # Keep the canvas constant if a flow resizes the window while recording.
    filters = (f"scale=iw:ih:eval=frame,pad={width}:{height}:0:0:eval=frame,"
               "tpad=stop_mode=clone:stop_duration=0.5,"
               "split[a][b];[a]palettegen[p];[b][p]paletteuse")
    subprocess.run(
        [ffmpeg, "-nostdin", "-hide_banner", "-loglevel", "error", "-n", "-reinit_filter", "0",
         "-framerate", str(RECORD_FPS), "-start_number", "1", "-i",
         str(recorder.directory / "frame-%06d.png"), "-filter_complex", filters,
         "-loop", "0", str(output)],
        check=True, timeout=30,
    )


def save_playback(ffmpeg, recorder, output, *, required):
    try:
        if recorder.error is not None:
            if required:
                raise RuntimeError("Spix frame capture stopped early") from recorder.error
            print(f"Spix frame capture stopped early: {recorder.error}", flush=True)
        encode_gif(ffmpeg, recorder, output)
    except (OSError, RuntimeError, subprocess.SubprocessError) as error:
        output.unlink(missing_ok=True)
        if required:
            raise
        print(f"Could not encode the GIF: {error}", flush=True)
    else:
        print(f"Recorded {output}", flush=True)


def run_flow_cli(suite, flow, description, *, fixture=False, arguments=()):
    parser = argparse.ArgumentParser(description=description)
    parser.add_argument("executable", type=Path, help="desktop executable built with Spix")
    parser.add_argument("--gif", type=Path, help="record a playback GIF using FFmpeg")
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

    temporary = tempfile.TemporaryDirectory(prefix="spix-") if ffmpeg else nullcontext()
    with temporary as scratch:
        recorder = FrameRecorder(Path(scratch)) if scratch else None
        artifacts = Path(report) / suite if report else Path(scratch) if scratch else None
        if artifacts is not None:
            artifacts.mkdir(exist_ok=True)
            # CTest can repeat a flow within the same report. Keep the last attempt.
            for path in artifacts.glob("step-*.png"):
                path.unlink()
            for name in ("app.log", "failure.png", "playback.gif"):
                (artifacts / name).unlink(missing_ok=True)
        output = args.gif or (artifacts / "playback.gif" if recorder and artifacts else None)
        try:
            startup = ("--fake-api", *arguments) if fixture else arguments
            with desktop_session(args.executable, startup,
                                 artifacts=artifacts, capture_ui=fixture, recorder=recorder) as session:
                flow(session)
        except Exception:
            if output is not None:
                save_playback(ffmpeg, recorder, output, required=False)
            raise
        if output is not None:
            save_playback(ffmpeg, recorder, output, required=required)
