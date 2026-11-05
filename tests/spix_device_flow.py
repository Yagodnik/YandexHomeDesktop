#!/usr/bin/env python3
"""Exercise household selection, Favorites, and brightness through Spix."""

from spix_harness import WINDOW, run_fixture_cli


PICKER = f"{WINDOW}/householdPicker"
HOME_LAMP = f"{WINDOW}/device-lamp"
CABIN_LAMP = f"{WINDOW}/roomDevices-cabin-room/device-cabin-lamp"
FAVORITE_LAMP = f"{WINDOW}/favoriteDevicesList/device-cabin-lamp"
BRIGHTNESS = f"{WINDOW}/deviceRangeSlider"


def run_flow(session):
    rpc = session.rpc
    session.wait_for(lambda: rpc.existsAndVisible(HOME_LAMP), "the Home household", timeout=20)
    session.checkpoint()

    cabin_option = f"{PICKER}/household-cabin"
    rpc.mouseClick(f"{WINDOW}/householdSelector")
    session.wait_for(
        lambda: rpc.getStringProperty(PICKER, "opened") == "true"
        and session.in_window(cabin_option),
        "the Cabin picker row to slide into view",
    )
    session.checkpoint()
    rpc.mouseClick(cabin_option)
    session.wait_for(
        lambda: rpc.existsAndVisible(CABIN_LAMP)
        and not rpc.existsAndVisible(HOME_LAMP)
        and rpc.getStringProperty(PICKER, "opened") == "false",
        "the filtered Cabin device list",
    )
    session.checkpoint("Cabin selected and Home devices filtered out")

    if rpc.existsAndVisible(FAVORITE_LAMP):
        raise RuntimeError("Cabin lamp was already in Favorites")
    star = f"{CABIN_LAMP}/favorite-cabin-lamp"
    session.wait_for(lambda: rpc.getStringProperty(star, "enabled") == "true",
                     "the Cabin lamp favorite button to become enabled")
    rpc.mouseClick(star)
    session.wait_for(lambda: rpc.existsAndVisible(FAVORITE_LAMP), "Cabin lamp in Favorites")
    session.checkpoint("Cabin lamp added to Favorites")

    rpc.mouseClick(FAVORITE_LAMP)
    session.wait_for(lambda: session.page_ready(BRIGHTNESS), "the lamp brightness slider")
    before = float(rpc.getStringProperty(BRIGHTNESS, "value"))
    if before != 45:
        raise RuntimeError(f"Expected initial fixture brightness of 45%, got {before:g}%")
    session.checkpoint()

    rpc.mouseClickWithProportion(BRIGHTNESS, 0.8, 0.5)
    session.wait_for(lambda: 78 <= float(rpc.getStringProperty(BRIGHTNESS, "value")) <= 82,
                     "brightness near 80%")
    after = float(rpc.getStringProperty(BRIGHTNESS, "value"))
    session.checkpoint(seconds=1)

    rpc.mouseClick(f"{WINDOW}/#DeviceHeader/backButton")
    session.wait_for(lambda: session.page_ready(FAVORITE_LAMP)
                     and not rpc.existsAndVisible(BRIGHTNESS), "the Favorites list after going back")
    session.checkpoint()
    rpc.mouseClick(FAVORITE_LAMP)
    session.wait_for(lambda: session.page_ready(BRIGHTNESS), "the reopened lamp controls")
    actual = float(rpc.getStringProperty(BRIGHTNESS, "value"))
    if actual != after:
        raise RuntimeError(f"Reopened brightness was {actual:g}%, expected {after:g}%")
    session.checkpoint(f"Brightness changed from {before:g}% to {after:g}% and survived reopening",
                       seconds=1.5)


if __name__ == "__main__":
    run_fixture_cli(run_flow, __doc__)
