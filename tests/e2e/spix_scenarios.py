#!/usr/bin/env python3
"""Exercise scenario controls with misses, rapid clicks, and failure recovery."""

import time

from spix_harness import WINDOW, run_flow_cli


TAB = f"{WINDOW}/scenariosTab"
EVENING = f"{WINDOW}/scenarioDelegate_evening"
INACTIVE = f"{WINDOW}/scenarioDelegate_inactive"
FAILURE = f"{WINDOW}/scenarioDelegate_failure"
DISMISS = f"{WINDOW}/errorDismissAction"


def run_flow(session):
    rpc = session.rpc

    def start(row):
        return f"{row}/scenarioStart"

    def pending(row):
        return rpc.getStringProperty(row, "is_waiting_response") == "true"

    session.wait_for(lambda: session.page_ready(TAB), "the Scenarios tab", timeout=20)
    rpc.mouseClick(TAB)
    session.wait_for(lambda: session.page_ready(start(EVENING))
                     and rpc.existsAndVisible(start(INACTIVE)) and session.in_window(start(INACTIVE)),
                     "the active and inactive scenario controls")

    rpc.mouseClick(f"{EVENING}/scenarioName")
    session.click_burst(start(INACTIVE), start(INACTIVE), start(INACTIVE))
    if pending(EVENING) or pending(INACTIVE) or rpc.existsAndVisible(DISMISS):
        raise RuntimeError("Clicking the title or inactive play control started an action")
    session.checkpoint("The scenario title and inactive play control ignored clicks")

    session.click_burst(start(FAILURE), start(FAILURE), start(FAILURE))
    session.wait_for(lambda: session.page_ready(DISMISS) and session.in_window(DISMISS),
                     "the execution error after rapid clicks")
    # Let the dialog finish opening while the recorder continues to sample it.
    time.sleep(0.2)
    session.checkpoint("Rapid clicks on the failing scenario led to a dismissible error")
    rpc.mouseClick(DISMISS)
    session.wait_for(lambda: not rpc.existsAndVisible(DISMISS) and not pending(FAILURE),
                     "the closed error and settled scenario")

    session.click_burst(start(EVENING), start(EVENING), start(EVENING))
    if not pending(EVENING):
        raise RuntimeError("The active scenario did not enter its pending state after the error")
    session.wait_for(lambda: not pending(EVENING) and rpc.existsAndVisible(start(EVENING)),
                     "the active scenario to finish")
    if rpc.existsAndVisible(DISMISS):
        raise RuntimeError("An error appeared after the active scenario completed")
    session.checkpoint("The active scenario ran successfully after dismissing the error")


if __name__ == "__main__":
    run_flow_cli("ScenarioExecution", run_flow, __doc__, fixture=True)
