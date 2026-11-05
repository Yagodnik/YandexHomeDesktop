#!/usr/bin/env python3
"""Exercise the real desktop window through the opt-in Spix listener."""

from spix_harness import WINDOW, run_flow_cli


def run_flow(session):
    title = "Spix smoke test"
    session.rpc.setStringProperty(WINDOW, "title", title)
    if session.rpc.getStringProperty(WINDOW, "title") != title:
        raise RuntimeError("Spix did not update the QML window title")
    session.checkpoint("Window title updated through Spix")


if __name__ == "__main__":
    run_flow_cli("SpixSmoke", run_flow, __doc__)
