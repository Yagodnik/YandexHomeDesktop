#!/usr/bin/env python3
"""Use the local REST API with Python's standard library (Python 3.9+)."""

import argparse
import json
import subprocess
import sys
from urllib.error import HTTPError, URLError
from urllib.parse import quote
from urllib.request import ProxyHandler, Request, build_opener


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", default="YandexHomeCli", help="Path to the console executable")
    parser.add_argument("--fake-api", action="store_true", help="Connect to the Debug fixture server")
    parser.add_argument("--fake-api-data", help="Use the same fixture path used to enable REST")
    parser.add_argument("--device-id", help="Read a device and optionally change its power state")
    power = parser.add_mutually_exclusive_group()
    power.add_argument("--on", action="store_true")
    power.add_argument("--off", action="store_true")
    parser.add_argument("--scenario-id", help="Run an active scenario")
    args = parser.parse_args()
    if (args.on or args.off) and not args.device_id:
        parser.error("--on/--off requires --device-id")
    if args.fake_api_data and not args.fake_api:
        parser.error("--fake-api-data requires --fake-api")

    cli = [args.cli]
    if args.fake_api:
        cli.append("--fake-api")
    if args.fake_api_data:
        cli.extend(["--fake-api-data", args.fake_api_data])

    def control(command):
        result = subprocess.run(cli + [command, "--json"], capture_output=True, text=True, encoding="utf-8", check=False)
        if result.returncode:
            raise RuntimeError(result.stderr.strip())
        return json.loads(result.stdout)

    status = control("--status-rest")
    if not status["running"]:
        raise RuntimeError("Start REST with --enable-rest before running this example")
    opener = build_opener(ProxyHandler({}))

    def request(path, body=None):
        data = None if body is None else json.dumps(body).encode("utf-8")
        req = Request(status["url"] + path, data=data, headers={
            "Content-Type": "application/json",
        }, method="GET" if body is None else "POST")
        try:
            with opener.open(req, timeout=35) as response:
                result = json.load(response)
        except HTTPError as error:
            result = json.load(error)
            raise RuntimeError(f"HTTP {error.code}: {result['error']['message']}") from error
        print(json.dumps(result, ensure_ascii=False, indent=2))
        return result

    request("/devices")
    if args.device_id:
        device = quote(args.device_id, safe="")
        request("/devices/" + device)
        if args.on or args.off:
            request("/devices/" + device + "/actions", {
                "capability": "on_off", "state": {"instance": "on", "value": args.on},
            })
    if args.scenario_id:
        request("/scenarios/" + quote(args.scenario_id, safe="") + "/run", {})


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, RuntimeError, URLError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(1)
