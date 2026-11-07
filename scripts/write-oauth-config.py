"""Write the build-time OAuth resource without printing its contents."""

import argparse
import json
import os
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--dummy", action="store_true")
    source.add_argument("--from-env", action="store_true")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if args.dummy:
        config = {
            "auth_url": "https://example.invalid/authorize",
            "access_token_url": "https://example.invalid/token",
            "client_id": "ci-build-only",
            "client_secret": "",
            "redirect_base": "http://127.0.0.1",
            "redirect_port": 1337,
            "scopes": [],
        }
    else:
        raw = os.environ.get("RELEASE_OAUTH_CONFIG_JSON")
        if not raw:
            parser.error("RELEASE_OAUTH_CONFIG_JSON is required for release builds")
        try:
            config = json.loads(raw)
        except json.JSONDecodeError as exc:
            parser.error(f"RELEASE_OAUTH_CONFIG_JSON is invalid JSON: {exc.msg}")

    required_strings = ("auth_url", "access_token_url", "client_id", "redirect_base")
    if not isinstance(config, dict) or any(
        not isinstance(config.get(key), str) or not config[key]
        for key in required_strings
    ):
        parser.error("OAuth config must contain nonempty auth_url, access_token_url, client_id, and redirect_base strings")
    if config.get("client_secret", "") != "":
        parser.error("client_secret must be empty because it would be embedded in the public app")
    if type(config.get("redirect_port")) is not int or not 1 <= config["redirect_port"] <= 65535:
        parser.error("redirect_port must be an integer from 1 to 65535")
    scopes = config.get("scopes")
    if not isinstance(scopes, list) or not all(isinstance(scope, str) for scope in scopes):
        parser.error("scopes must be a list of strings")

    config["client_secret"] = ""
    args.output.parent.mkdir(parents=True, exist_ok=True)
    flags = os.O_WRONLY | os.O_CREAT | os.O_TRUNC
    fd = os.open(args.output, flags, 0o600)
    with os.fdopen(fd, "w", encoding="utf-8") as output:
        json.dump(config, output, separators=(",", ":"))
        output.write("\n")


if __name__ == "__main__":
    main()
