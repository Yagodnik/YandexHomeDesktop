#!/usr/bin/env python3
"""Exercise picker dismissal and household filtering through the desktop."""

from spix_harness import WINDOW, run_flow_cli


SELECTOR = f"{WINDOW}/householdSelector"
PICKER = f"{WINDOW}/householdPicker"
SHEET = f"{PICKER}/#SelectDialog"
SCENARIOS = f"{WINDOW}/scenariosTab"
HOME_LAMP = f"{WINDOW}/roomDevices-office/device-lamp"
CABIN_LAMP = f"{WINDOW}/roomDevices-cabin-room/device-cabin-lamp"


def run_flow(session):
    rpc = session.rpc

    def selected(household, visible_lamp, hidden_lamp):
        return (rpc.getStringProperty(PICKER, "opened") == "false"
                and rpc.getStringProperty(SHEET, "opacity") == "0"
                and rpc.getStringProperty(PICKER, "currentHousehold") == household
                and session.page_ready(visible_lamp)
                and not rpc.existsAndVisible(hidden_lamp))

    def picker_ready(option):
        return (rpc.getStringProperty(PICKER, "opened") == "true"
                and rpc.getStringProperty(SHEET, "opacity") == "1"
                and session.in_window(option))

    def open_picker(option):
        rpc.mouseClick(SELECTOR)
        session.wait_for(lambda: picker_ready(option), "the household option inside the picker")

    session.wait_for(lambda: session.page_ready(SELECTOR) and selected("home", HOME_LAMP, CABIN_LAMP),
                     "the Home device list", timeout=20)
    cabin = f"{PICKER}/household-cabin"
    open_picker(cabin)

    # The tab's coordinates are covered by the backdrop while the picker is open.
    rpc.mouseClick(SCENARIOS)
    session.wait_for(lambda: selected("home", HOME_LAMP, CABIN_LAMP),
                     "dismissal without switching tabs or households")
    session.checkpoint("Clicking outside the picker kept Home and Devices selected")

    open_picker(cabin)
    rpc.mouseClick(cabin)
    session.wait_for(lambda: selected("cabin", CABIN_LAMP, HOME_LAMP), "the filtered Cabin list")
    session.checkpoint("Cabin selection replaced the Home devices")

    # The first selector click opens the picker, the second lands on its backdrop,
    # and the third reopens it.
    session.click_burst(SELECTOR, SELECTOR, SELECTOR)
    home = f"{PICKER}/household-home"
    session.wait_for(lambda: picker_ready(home), "the reopened picker after rapid selector clicks")
    rpc.mouseClick(home)
    session.wait_for(lambda: selected("home", HOME_LAMP, CABIN_LAMP), "the restored Home list")
    session.checkpoint("Rapid picker reopening still allowed switching back to Home")


if __name__ == "__main__":
    run_flow_cli("HouseholdPicker", run_flow, __doc__, fixture=True)
