#!/usr/bin/env python3
"""Exercise the real desktop window through the opt-in Spix listener."""

import os
from pathlib import Path
import socket
import subprocess
import sys
import time
import xmlrpc.client


class TimedTransport(xmlrpc.client.Transport):
    def make_connection(self, host):
        connection = super().make_connection(host)
        connection.timeout = 3
        return connection


def main():
    executable = Path(sys.argv[1]).resolve(strict=True)
    with socket.socket() as probe:
        probe.settimeout(1)
        if probe.connect_ex(("127.0.0.1", 9000)) == 0:
            raise RuntimeError("Spix smoke test needs port 9000 to be free")
    env = dict(
        os.environ,
        QT_QPA_PLATFORM="offscreen",
        QT_QUICK_BACKEND="software",
        QT_QPA_FONTDIR=str(Path(__file__).resolve().parents[1] / "resources" / "fonts"),
    )
    with subprocess.Popen([str(executable), *sys.argv[2:]], env=env) as app:
        try:
            with xmlrpc.client.ServerProxy(
                "http://127.0.0.1:9000", transport=TimedTransport()
            ) as rpc:
                deadline = time.monotonic() + 20
                while True:
                    if app.poll() is not None:
                        raise RuntimeError(f"Desktop app exited with status {app.returncode}")
                    try:
                        if rpc.getStringProperty("mainWindow", "title"):
                            break
                    except OSError:
                        pass
                    if time.monotonic() >= deadline:
                        raise RuntimeError("Spix did not find the desktop window")
                    time.sleep(0.1)

                title = "Spix smoke test"
                rpc.setStringProperty("mainWindow", "title", title)
                if rpc.getStringProperty("mainWindow", "title") != title:
                    raise RuntimeError("Spix did not update the QML window title")
                rpc.quit()

            if app.wait(timeout=10) != 0:
                raise RuntimeError(f"Desktop app exited with status {app.returncode}")
        finally:
            if app.poll() is None:
                app.kill()


if __name__ == "__main__":
    main()
