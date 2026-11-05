#!/usr/bin/env python3
"""Exercise cancellation, invalid confirmation, and retry using local sign-in."""

from http.client import HTTPConnection
import re
from urllib.parse import urlencode

from spix_harness import WINDOW, run_flow_cli


SIGN_IN = f"{WINDOW}/signInAction"
CANCEL = f"{WINDOW}/cancelSignInAction"
MESSAGE = f"{WINDOW}/signInMessage"
HOME_LAMP = f"{WINDOW}/roomDevices-office/device-lamp"
CHALLENGE = re.compile(r"http://127\.0\.0\.1:(\d+)/.*?\b([A-Z]+-\d+)\b", re.DOTALL)


def confirm(port, code):
    connection = HTTPConnection("127.0.0.1", port, timeout=3)
    try:
        connection.request("GET", f"/confirm?{urlencode({'code': code})}")
        response = connection.getresponse()
        if response.status != 200:
            raise RuntimeError(f"Fixture confirmation returned HTTP {response.status}")
        response.read()
    finally:
        connection.close()


def run_flow(session):
    rpc = session.rpc

    def begin_sign_in():
        rpc.mouseClick(SIGN_IN)
        session.wait_for(lambda: session.page_ready(CANCEL) and session.in_window(CANCEL),
                         "the pending local sign-in")
        message = rpc.getStringProperty(MESSAGE, "text")
        challenge = CHALLENGE.search(message)
        if challenge is None:
            raise RuntimeError(f"Could not read the fixture URL and code from {message!r}")
        return int(challenge[1]), challenge[2]

    session.wait_for(lambda: session.page_ready(SIGN_IN) and session.in_window(SIGN_IN),
                     "the signed-out page", timeout=20)
    rpc.mouseClickWithProportion(WINDOW, 0.05, 0.1)
    rpc.wait(0)
    if rpc.existsAndVisible(CANCEL):
        raise RuntimeError("A click outside the sign-in card began authorization")

    canceled_port, canceled_code = begin_sign_in()
    rpc.mouseClick(CANCEL)
    session.wait_for(lambda: session.page_ready(SIGN_IN) and not rpc.existsAndVisible(CANCEL),
                     "the signed-out page after cancellation")
    try:
        confirm(canceled_port, canceled_code)
    except ConnectionRefusedError:
        pass
    else:
        raise RuntimeError("The canceled challenge still accepted connections")
    session.checkpoint("Cancel closed the original sign-in challenge")

    port, code = begin_sign_in()
    confirm(port, "invalid-code")
    if not rpc.existsAndVisible(CANCEL) or rpc.existsAndVisible(HOME_LAMP):
        raise RuntimeError("An invalid confirmation left the pending sign-in state")
    session.checkpoint("An invalid code kept the new challenge pending")

    confirm(port, code)
    session.wait_for(lambda: session.page_ready(HOME_LAMP) and not rpc.existsAndVisible(CANCEL),
                     "the Home device list after confirming the retry")
    session.checkpoint("Confirming the new challenge opened Home")


if __name__ == "__main__":
    run_flow_cli("FixtureSignIn", run_flow, __doc__, fixture=True, arguments=("--fake-auth",))
