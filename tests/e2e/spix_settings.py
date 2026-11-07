#!/usr/bin/env python3
"""Exercise missed clicks, rapid toggles, and tab changes in Settings."""

from spix_harness import WINDOW, run_flow_cli


SETTINGS = f"{WINDOW}/settingsTab"
DEVICES = f"{WINDOW}/devicesTab"
SCENARIOS = f"{WINDOW}/scenariosTab"
KEYBIND_ROW = f"{WINDOW}/keybindsSettingRow"
KEYBIND_SWITCH = f"{KEYBIND_ROW}/settingSwitch"
EVENING = f"{WINDOW}/scenarioDelegate_evening"
SCENARIO_KEYBIND = f"{EVENING}/scenarioKeybind"


def run_flow(session):
    rpc = session.rpc
    # Give the Settings card enough room even on hosts that show the notch row.
    rpc.setStringProperty(WINDOW, "height", "620")
    session.wait_for(lambda: session.page_ready(SETTINGS), "the Settings tab", timeout=20)
    rpc.mouseClick(SETTINGS)
    session.wait_for(lambda: session.page_ready(KEYBIND_SWITCH) and session.in_window(KEYBIND_SWITCH),
                     "the keybind visibility switch")

    initial = rpc.getStringProperty(KEYBIND_SWITCH, "checked")
    if initial not in ("true", "false"):
        raise RuntimeError(f"Unexpected initial switch state: {initial!r}")
    opposite = "false" if initial == "true" else "true"

    rpc.mouseClickWithProportion(KEYBIND_ROW, 0.2, 0.5)
    rpc.wait(0)
    if rpc.getStringProperty(KEYBIND_SWITCH, "checked") != initial:
        raise RuntimeError("Clicking the row label toggled the switch")
    session.checkpoint("A missed switch click preserved the setting")

    session.click_burst(KEYBIND_SWITCH, KEYBIND_SWITCH)
    if rpc.getStringProperty(KEYBIND_SWITCH, "checked") != initial:
        raise RuntimeError("Two rapid switch clicks did not restore the original value")

    for expected in (opposite, initial):
        session.click_burst(KEYBIND_SWITCH, KEYBIND_SWITCH, KEYBIND_SWITCH)
        session.wait_for(lambda: rpc.getStringProperty(KEYBIND_SWITCH, "checked") == expected,
                         f"the switch state to settle on {expected}")
        session.click_burst(DEVICES, SETTINGS, SCENARIOS, SCENARIOS)
        session.wait_for(lambda: session.page_ready(EVENING)
                         and rpc.existsAndVisible(SCENARIO_KEYBIND) == (expected == "true"),
                         f"the scenario shortcut visibility to match {expected}")
        session.checkpoint(f"Scenario shortcut visibility matched the setting ({expected})")
        rpc.mouseClick(SETTINGS)
        session.wait_for(lambda: session.page_ready(KEYBIND_SWITCH)
                         and rpc.getStringProperty(KEYBIND_SWITCH, "checked") == expected,
                         "the setting to survive leaving and returning to Settings")


if __name__ == "__main__":
    run_flow_cli("KeybindVisibility", run_flow, __doc__, fixture=True)
