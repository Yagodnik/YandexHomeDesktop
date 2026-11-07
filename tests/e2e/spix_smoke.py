#!/usr/bin/env python3
"""Exercise the real desktop window through the opt-in Spix listener."""

import os
from pathlib import Path
import sys
import tempfile

from spix_harness import WINDOW, run_flow_cli


def run_flow(session):
    if sys.platform == "linux":
        session.wait_for(lambda: session.rpc.getStringProperty(WINDOW, "visible") == "true",
                         "a normal window with the saved tray preference in a session without a tray")
        if session.rpc.getStringProperty(WINDOW, "trayModeEnabled") != "false":
            raise RuntimeError("Tray mode was enabled without a system tray")
    title = "Spix smoke test"
    session.rpc.setStringProperty(WINDOW, "title", title)
    if session.rpc.getStringProperty(WINDOW, "title") != title:
        raise RuntimeError("Spix did not update the QML window title")
    session.checkpoint("Window title updated through Spix")


if __name__ == "__main__":
    if sys.platform == "linux":
        with tempfile.TemporaryDirectory(prefix="yh-spix-linux-") as scratch:
            config = Path(scratch) / "ArtemYagodnik" / "YandexHomeDesktop.conf"
            config.parent.mkdir()
            config.write_text("[General]\ntrayModeEnabled=true\n", encoding="utf-8")
            os.environ["XDG_CONFIG_HOME"] = scratch
            os.environ["DBUS_SESSION_BUS_ADDRESS"] = f"unix:path={scratch}/no-session-bus"
            run_flow_cli("SpixSmoke", run_flow, __doc__)
    else:
        run_flow_cli("SpixSmoke", run_flow, __doc__)
