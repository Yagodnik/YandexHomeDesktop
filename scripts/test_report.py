#!/usr/bin/env python3
"""Run CTest and write an offline HTML report beside its logs and artifacts."""

import argparse
from html import escape
import json
import os
from pathlib import Path
import subprocess
import tempfile
from urllib.parse import quote
import xml.etree.ElementTree as ET


CATALOG = Path(__file__).resolve().parents[1] / "tests" / "catalog.json"


def link(path, directory, label):
    url = quote(path.relative_to(directory).as_posix())
    return f'<a href="{url}">{escape(label)}</a>'


def write_report(directory, catalog_path=CATALOG):
    suites = json.loads(catalog_path.read_text(encoding="utf-8"))["suites"]
    descriptions = {suite["id"]: suite["description"] for suite in suites}
    result = directory / "results.xml"
    rows = []
    if result.is_file():
        for index, test in enumerate(ET.parse(result).iter("testcase"), 1):
            name = test.get("name", "")
            description = descriptions.get(name, "")
            title = escape(name)
            if description:
                title += f'<div class="description">{escape(description)}</div>'
            status = {"run": "passed", "notrun": "not run"}.get(test.get("status"), "unknown")
            labels = test.find('properties/property[@name="cmake_labels"]')
            tags = ", ".join(labels.get("value", "").split(";")) if labels is not None else ""
            failure = test.find("failure")
            if failure is not None:
                status = "failed"
            elif test.find("skipped") is not None:
                status = "skipped"
            output = directory / f"output-{index}.txt"
            details = [test.findtext("system-out"), failure.text if failure is not None else None]
            output.write_text("\n".join(part for part in details if part), encoding="utf-8")
            files = [link(output, directory, "Test output")]
            # Catalog IDs are path-safe. Never traverse arbitrary JUnit names.
            if name.isascii() and name.replace("_", "").isalnum():
                artifacts = directory / name
                for filename, label in (("app.log", "App log"), ("failure.png", "Failure screenshot"),
                                        ("playback.gif", "Open GIF")):
                    path = artifacts / filename
                    if path.is_file():
                        files.append(link(path, directory, label))
                files.extend(link(path, directory, path.stem) for path in sorted(artifacts.glob("step-*.png")))
            duration = test.get("time", "")
            rows.append(f"<tr><td>{title}</td><td>{escape(tags)}</td><td>{escape(status)}</td>"
                        f"<td>{escape(duration)}</td><td>{' · '.join(files)}</td></tr>")
    if not rows:
        rows.append('<tr><td colspan="5">No test results were produced.</td></tr>')
    (directory / "index.html").write_text(
        '<!doctype html><html lang="en"><meta charset="utf-8">'
        '<meta name="viewport" content="width=device-width, initial-scale=1">'
        '<title>Test report</title><style>'
        'body{font:16px system-ui,sans-serif;margin:2rem}table{border-collapse:collapse}'
        'td,th{text-align:left;vertical-align:top;padding:.5rem;border-bottom:1px solid #888}'
        '.description{font-size:.9em;margin-top:.25em;max-width:48ch}'
        '</style><h1>Test report</h1><p><a href="ctest.log">CTest output</a></p>'
        '<table><thead><tr><th>Test</th><th>Labels</th><th>Status</th><th>Seconds</th><th>Files</th>'
        '</tr></thead><tbody>' + "\n".join(rows) + '</tbody></table></html>',
        encoding="utf-8",
    )


def run_with_report(command, build_dir, *, record=False, catalog_path=CATALOG):
    report_root = build_dir / "test-reports"
    report_root.mkdir(exist_ok=True)
    directory = Path(tempfile.mkdtemp(prefix="run-", dir=report_root))
    environment = dict(os.environ, YH_TEST_ARTIFACT_DIR=str(directory),
                       YH_TEST_RECORD_GIF="1" if record else "0")
    command = command + ["--output-junit", str(directory / "results.xml"),
                         "--output-log", str(directory / "ctest.log")]
    status = subprocess.run(command, env=environment).returncode
    try:
        write_report(directory, catalog_path)
    except (ET.ParseError, OSError, json.JSONDecodeError) as error:
        print(f"Could not write test report: {error}", flush=True)
        return status or 2
    print(f"Test report: {directory / 'index.html'}", flush=True)
    return status


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", type=Path)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    return run_with_report(args.command, args.build_dir.resolve())


if __name__ == "__main__":
    raise SystemExit(main())
