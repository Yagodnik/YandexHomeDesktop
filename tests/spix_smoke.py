#!/usr/bin/env python3
"""Exercise the real desktop window through the opt-in Spix listener."""

import sys

from spix_harness import WINDOW, desktop_session


def main():
    with desktop_session(sys.argv[1], sys.argv[2:]) as session:
        title = "Spix smoke test"
        session.rpc.setStringProperty(WINDOW, "title", title)
        if session.rpc.getStringProperty(WINDOW, "title") != title:
            raise RuntimeError("Spix did not update the QML window title")


if __name__ == "__main__":
    main()
