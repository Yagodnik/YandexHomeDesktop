#!/usr/bin/env python3
"""Exercise repeated star clicks and Favorites collapse in the desktop."""

from spix_harness import WINDOW, run_flow_cli


SOCKET = f"{WINDOW}/roomDevices-living/device-socket"
ROOM_STAR = f"{SOCKET}/favorite-socket"
FAVORITES = f"{WINDOW}/favoriteDevicesList"
FAVORITE_SOCKET = f"{FAVORITES}/device-socket"
FAVORITES_HEADER = f"{WINDOW}/collapse-favorites"
FAVORITES_BODY = f"{WINDOW}/sectionBody-favorites"


def run_flow(session):
    rpc = session.rpc

    def favorite_state(count, starred):
        return (rpc.getStringProperty(FAVORITES, "count") == str(count)
                and rpc.getStringProperty(ROOM_STAR, "favorite") == starred)

    session.wait_for(lambda: session.page_ready(ROOM_STAR) and session.in_window(ROOM_STAR)
                     and rpc.getStringProperty(ROOM_STAR, "enabled") == "true"
                     and favorite_state(0, "false"), "the unstarred Living room socket", timeout=20)
    session.checkpoint("The Living room socket started outside Favorites")

    session.click_burst(ROOM_STAR, ROOM_STAR, ROOM_STAR)
    session.wait_for(lambda: favorite_state(1, "true") and session.page_ready(FAVORITE_SOCKET)
                     and not rpc.existsAndVisible(f"{WINDOW}/deviceControlsPane"),
                     "one favorite and no device navigation after three star clicks")
    session.checkpoint("Three rapid star clicks left one favorite")

    rpc.mouseClick(FAVORITES_HEADER)
    session.wait_for(lambda: not rpc.existsAndVisible(FAVORITES_BODY) and favorite_state(1, "true"),
                     "the collapsed Favorites section with its selection preserved")
    session.click_burst(FAVORITES_HEADER, FAVORITES_HEADER, FAVORITES_HEADER)
    favorite_star = f"{FAVORITE_SOCKET}/favorite-socket"
    session.wait_for(lambda: rpc.existsAndVisible(favorite_star) and session.in_window(favorite_star)
                     and rpc.getStringProperty(FAVORITES_BODY, "opacity") == "1",
                     "the reopened Favorites section after rapid header clicks")
    session.checkpoint("Rapid collapse and reopen preserved the favorite")

    rpc.mouseClick(favorite_star)
    session.wait_for(lambda: favorite_state(0, "false") and not rpc.existsAndVisible(FAVORITE_SOCKET),
                     "removal from Favorites and the matching room star to clear")
    session.checkpoint("Removing from Favorites cleared the room star")


if __name__ == "__main__":
    run_flow_cli("Favorites", run_flow, __doc__, fixture=True)
