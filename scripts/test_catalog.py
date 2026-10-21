#!/usr/bin/env python3
"""Describe and select repository tests; delegate execution to CTest/Qt Test."""

import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
CATALOG = ROOT / "tests" / "catalog.json"
KINDS = {"qt", "quick", "ctest"}
REQUIREMENTS = {"BUILD_DESKTOP_APP", "BUILD_CLI_APP", "BUILD_REST_APP", "BUILD_SHORTCUTS_APP", "YH_BUILD_REST_ADAPTER", "Debug"}


class CatalogError(ValueError):
    pass


def load_catalog(path=CATALOG):
    catalog = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(catalog, dict) or catalog.get("version") != 1:
        raise CatalogError("Unsupported catalog version (expected 1)")
    suites = catalog.get("suites")
    if not isinstance(suites, list) or not suites:
        raise CatalogError("Catalog needs a nonempty suites array")
    ids = set()
    for suite in suites:
        if not isinstance(suite, dict):
            raise CatalogError("Suite entries must be objects")
        name = suite.get("id", "")
        if not isinstance(name, str) or not re.fullmatch(r"[A-Za-z][A-Za-z0-9_]*", name):
            raise CatalogError(f"Invalid suite ID: {name!r}")
        if name in ids:
            raise CatalogError(f"Duplicate suite ID: {name}")
        ids.add(name)
        if suite.get("kind") not in KINDS:
            raise CatalogError(f"Invalid runner kind for {name}")
        for key in ("description", "layer", "source"):
            if not isinstance(suite.get(key), str) or not suite[key].strip():
                raise CatalogError(f"{name} needs {key}")
        source = (ROOT / suite["source"]).resolve()
        if ROOT not in source.parents or not source.is_file():
            raise CatalogError(f"{name} has an invalid source: {suite['source']}")
        features = suite.get("features")
        if not isinstance(features, list) or not features or any(
            not isinstance(tag, str) or not re.fullmatch(r"[a-z][a-z0-9-]*", tag) for tag in features
        ) or len(set(features)) != len(features):
            raise CatalogError(f"{name} needs unique feature tags")
        validate_requirements(suite, name)
        cases = suite.get("cases", [])
        if not isinstance(cases, list) or (suite["kind"] != "ctest" and not cases):
            raise CatalogError(f"{name} needs test cases")
        if suite["kind"] == "ctest" and cases:
            raise CatalogError(f"CTest-only suite {name} cannot have Qt test cases")
        names = set()
        for case in cases:
            if not isinstance(case, dict):
                raise CatalogError(f"{name} case entries must be objects")
            function = case.get("name", "")
            pattern = r"[A-Za-z_][A-Za-z0-9_]*"
            if suite["kind"] == "quick":
                pattern += r"::test_[A-Za-z0-9_]+"
            if not isinstance(function, str) or not re.fullmatch(pattern, function):
                raise CatalogError(f"Invalid case name in {name}: {function!r}")
            if function in names:
                raise CatalogError(f"Duplicate case: {name}::{function}")
            names.add(function)
            if not isinstance(case.get("description"), str) or not case["description"].strip():
                raise CatalogError(f"{name}::{function} needs a description")
            if "source" in case:
                if not isinstance(case["source"], str) or not case["source"].strip():
                    raise CatalogError(f"{name}::{function} has an invalid source")
                source = (ROOT / case["source"]).resolve()
                if ROOT not in source.parents or not source.is_file():
                    raise CatalogError(f"{name}::{function} has an invalid source")
            validate_requirements(case, f"{name}::{function}")
    return suites


def validate_requirements(entry, name):
    requirements = entry.get("requires", [])
    if not isinstance(requirements, list) or any(
        not isinstance(value, str) or value not in REQUIREMENTS for value in requirements
    ) or len(set(requirements)) != len(requirements):
        raise CatalogError(f"Invalid build requirements for {name}")


def read_profile(build_dir, configuration):
    cache = build_dir / "CMakeCache.txt"
    if not cache.is_file():
        raise CatalogError(f"No CMakeCache.txt in {build_dir}; configure the build first")
    values = {}
    for line in cache.read_text(encoding="utf-8").splitlines():
        if line.startswith(("//", "#")) or "=" not in line or ":" not in line:
            continue
        key, value = line.split("=", 1)
        values[key.split(":", 1)[0]] = value
    if Path(values.get("CMAKE_HOME_DIRECTORY", "")).resolve() != ROOT:
        raise CatalogError("Build directory belongs to a different checkout")
    if values.get("BUILD_TESTING") not in ("ON", "TRUE", "1", "YES"):
        raise CatalogError("Build directory has BUILD_TESTING disabled")
    configurations = values.get("CMAKE_CONFIGURATION_TYPES", "").split(";")
    effective = configuration or values.get("CMAKE_BUILD_TYPE", "")
    if configurations != [""]:
        if not effective or effective not in configurations:
            raise CatalogError("Select a configured build configuration with --config")
    elif configuration and configuration != values.get("CMAKE_BUILD_TYPE", ""):
        raise CatalogError(f"This build uses {values.get('CMAKE_BUILD_TYPE')}, not {configuration}")
    values["Debug"] = "ON" if effective == "Debug" else "OFF"
    return values, effective


def enabled(entry, profile):
    return profile is None or all(
        profile.get(requirement) in ("ON", "TRUE", "1", "YES")
        for requirement in entry.get("requires", [])
    )


def records(suites, profile=None, suites_only=False):
    result = []
    for suite in suites:
        entries = [None] if suites_only or not suite.get("cases") else suite["cases"]
        for case in entries:
            result.append({
                "id": suite["id"] + ("::" + case["name"] if case else ""),
                "suite": suite["id"],
                "name": case["name"] if case else suite["id"],
                "description": case["description"] if case else suite["description"],
                "features": suite["features"],
                "layer": suite["layer"],
                "source": case.get("source", suite["source"]) if case else suite["source"],
                "requires": suite.get("requires", []) + (case.get("requires", []) if case else []),
                "available": None if profile is None else enabled(suite, profile) and enabled(case or {}, profile),
            })
    return result


def filter_features(suites, features):
    known = {feature for suite in suites for feature in suite["features"]}
    unknown = set(features) - known
    if unknown:
        raise CatalogError("Unknown features: " + ", ".join(sorted(unknown)))
    return [suite for suite in suites if not features or set(features).intersection(suite["features"])]


def inventory(build_dir, configuration, ctest):
    command = [ctest, "--test-dir", str(build_dir), "--show-only=json-v1"]
    if configuration:
        command += ["-C", configuration]
    completed = subprocess.run(command, capture_output=True, text=True, timeout=30)
    if completed.returncode:
        raise CatalogError("CTest discovery failed:\n" + completed.stderr.strip())
    tests = json.loads(completed.stdout).get("tests", [])
    return {test["name"]: test for test in tests}


def properties(test):
    return {prop["name"]: prop["value"] for prop in test.get("properties", [])}


def discover_cases(test):
    props = properties(test)
    environment = os.environ.copy()
    for assignment in props.get("ENVIRONMENT", []):
        key, separator, value = assignment.partition("=")
        if not separator:
            raise CatalogError(f"Invalid test environment entry: {key}")
        environment[key] = value
    command = test.get("command")
    if not command or not Path(command[0]).is_file():
        raise CatalogError(f"{test['name']} executable is missing; build the tests first")
    if props.get("ENVIRONMENT_MODIFICATION"):
        raise CatalogError("Qt discovery needs support for ENVIRONMENT_MODIFICATION before using this property")
    # Qt opens -o files even for -functions. Discovery can run beside the suite,
    # so it must not truncate the execution log collected as a CI artifact.
    discovery_command = [command[0]]
    arguments = iter(command[1:])
    for argument in arguments:
        if argument == "-o":
            if next(arguments, None) is None:
                raise CatalogError(f"{test['name']} has an incomplete Qt output option")
        else:
            discovery_command.append(argument)
    completed = subprocess.run(
        discovery_command + ["-functions"], env=environment,
        cwd=props.get("WORKING_DIRECTORY"), capture_output=True, text=True,
        timeout=props.get("TIMEOUT", 30),
    )
    if completed.returncode:
        raise CatalogError(f"{test['name']} case discovery failed:\n" + completed.stderr.strip())
    # Qt Test writes to stdout; Qt Quick versions can use Qt's stderr logger.
    names = re.findall(r"^([A-Za-z_]\w*(?:::[A-Za-z_]\w*)?)\(\)\s*$",
                       completed.stdout + "\n" + completed.stderr, re.MULTILINE)
    if not names or len(names) != len(set(names)):
        raise CatalogError(f"{test['name']} returned empty or duplicate case names")
    return set(names)


def check_inventory(suites, tests, profile, selected=None):
    active = {suite["id"] for suite in suites if enabled(suite, profile)}
    if selected is None and active != set(tests):
        missing, extra = active - set(tests), set(tests) - active
        raise CatalogError(f"CTest/catalog suites differ; missing: {sorted(missing)}; uncatalogued: {sorted(extra)}. Reconfigure after catalog changes.")
    selected = active if selected is None else set(selected)
    for suite in suites:
        name = suite["id"]
        if name not in selected:
            continue
        if name not in tests:
            raise CatalogError(f"{name} is not registered in this build; reconfigure first")
        labels = set(properties(tests[name]).get("LABELS", []))
        if labels != set(suite["features"] + [suite["layer"]]):
            raise CatalogError(f"{name} labels differ from the catalog; reconfigure first")
        if suite["kind"] != "ctest":
            expected = {case["name"] for case in suite["cases"] if enabled(case, profile)}
            actual = discover_cases(tests[name])
            if actual != expected:
                raise CatalogError(f"{name} cases differ; missing: {sorted(expected - actual)}; undescribed: {sorted(actual - expected)}")


def select(suites, selectors, features, run_all, profile):
    if run_all and (selectors or features):
        raise CatalogError("Use --all on its own, or select IDs/features")
    if not run_all and not selectors and not features:
        raise CatalogError("Select test IDs, --feature, or --all")
    by_id = {suite["id"]: suite for suite in suites}
    # None means the full suite; a set means specific Qt functions.
    chosen = {}
    unavailable = []
    if run_all or features:
        for suite in filter_features(suites, features):
            if enabled(suite, profile):
                chosen[suite["id"]] = None
            else:
                unavailable.append(suite["id"])
    for selector in selectors:
        suite_id, separator, function = selector.partition("::")
        suite = by_id.get(suite_id)
        if suite is None:
            raise CatalogError(f"Unknown test ID: {selector}")
        if not enabled(suite, profile):
            raise CatalogError(f"{selector} is unavailable in this build (requires {suite.get('requires', [])})")
        if separator:
            case = next((case for case in suite.get("cases", []) if case["name"] == function), None)
            if case is None:
                raise CatalogError(f"Unknown test ID: {selector}")
            if not enabled(case, profile):
                raise CatalogError(f"{selector} is unavailable in this build (requires {case.get('requires', [])})")
            if suite_id not in chosen:
                chosen[suite_id] = set()
            if chosen[suite_id] is not None:
                chosen[suite_id].add(function)
        else:
            chosen[suite_id] = None
    if not chosen:
        raise CatalogError("Selection has no tests available in this build")
    return chosen, unavailable


def cmake_quote(value):
    value = str(value)
    equals = "="
    while "]" + equals + "]" in value:
        equals += "="
    return "[" + equals + "[" + value + "]" + equals + "]"


def write_selection(directory, tests, chosen):
    lines = ["# Generated test selection. Commands/properties come from CTest discovery."]
    for name, functions in chosen.items():
        test = tests[name]
        command = test.get("command")
        if not command:
            raise CatalogError(f"{name} command is missing; build the tests first")
        command = command + (sorted(functions) if functions is not None else [])
        lines.append("add_test(" + " ".join(cmake_quote(value) for value in [name] + command) + ")")
        for key, value in properties(test).items():
            if isinstance(value, list):
                value = ";".join(str(item).replace(";", "\\;") for item in value)
            elif isinstance(value, bool):
                value = "TRUE" if value else "FALSE"
            elif not isinstance(value, (str, int, float)):
                raise CatalogError(f"Unsupported CTest property {key} for {name}")
            lines.append(f"set_tests_properties({cmake_quote(name)} PROPERTIES {key} {cmake_quote(value)})")
    (directory / "CTestTestfile.cmake").write_text("\n".join(lines) + "\n", encoding="utf-8")


def run_selection(build_dir, configuration, ctest, tests, chosen, jobs):
    # CTest still enforces timeouts, environments, locks, and working directories.
    with tempfile.TemporaryDirectory(prefix="test-selection-", dir=build_dir) as temporary:
        directory = Path(temporary)
        write_selection(directory, tests, chosen)
        command = [ctest, "--test-dir", str(directory), "--output-on-failure", "--no-tests=error", "--parallel", str(jobs)]
        if configuration:
            command += ["-C", configuration]
        return subprocess.run(command).returncode


def parser():
    result = argparse.ArgumentParser(description=__doc__)
    commands = result.add_subparsers(dest="action", required=True)
    for name in ("list", "show", "check", "run"):
        command = commands.add_parser(name)
        command.add_argument("--catalog", type=Path, default=CATALOG)
        command.add_argument("--build-dir", type=Path)
        command.add_argument("--config", default="")
        command.add_argument("--ctest", default="ctest")
        if name in ("list", "show"):
            command.add_argument("--json", action="store_true")
        if name in ("list", "run"):
            command.add_argument("--feature", action="append", default=[])
        if name == "list":
            command.add_argument("--suites", action="store_true", help="List suites instead of individual cases")
            command.add_argument("--available", action="store_true", help="Only cases compiled into --build-dir")
        if name == "show":
            command.add_argument("id")
        if name == "run":
            command.add_argument("ids", nargs="*")
            command.add_argument("--all", action="store_true")
            command.add_argument("--dry-run", action="store_true")
            command.add_argument("--jobs", type=int, default=4)
    return result


def main(argv=None):
    args = parser().parse_args(argv)
    try:
        suites = load_catalog(args.catalog)
        profile, configuration = None, args.config
        build_dir = args.build_dir.resolve() if args.build_dir else None
        if build_dir:
            profile, configuration = read_profile(build_dir, args.config)
        if args.action == "list":
            if args.available and profile is None:
                raise CatalogError("--available requires --build-dir")
            items = records(filter_features(suites, args.feature), profile, args.suites)
            if args.available:
                items = [item for item in items if item["available"]]
        elif args.action == "show":
            items = [item for item in records(suites, profile, True) + records(suites, profile) if item["id"] == args.id]
            items = items[:1]
            if not items:
                raise CatalogError(f"Unknown test ID: {args.id}")
        elif args.action == "check":
            if build_dir:
                check_inventory(suites, inventory(build_dir, configuration, args.ctest), profile)
            print(f"Catalog valid: {len(suites)} suites, {len(records(suites))} test entries" + ("; CTest/Qt discovery matches" if build_dir else " (metadata only; use --build-dir to check discovery)"))
            return 0
        else:
            if build_dir is None:
                raise CatalogError("run requires --build-dir")
            if args.jobs < 1:
                raise CatalogError("--jobs must be positive")
            chosen, unavailable = select(suites, args.ids, args.feature, args.all, profile)
            tests = inventory(build_dir, configuration, args.ctest)
            check_inventory(suites, tests, profile, chosen)
            for name, functions in chosen.items():
                print("Selected: " + (name if functions is None else ", ".join(name + "::" + function for function in sorted(functions))), flush=True)
            if unavailable:
                print("Unavailable suites in this profile: " + ", ".join(unavailable), flush=True)
            return 0 if args.dry_run else run_selection(build_dir, configuration, args.ctest, tests, chosen, args.jobs)
        if args.json:
            print(json.dumps(items[0] if args.action == "show" else items, indent=2))
        else:
            for item in items:
                status = " [unavailable in this build]" if item["available"] is False else ""
                print(f"{item['id']}{status}\n  {item['description']}\n  {item['layer']}; features: {', '.join(item['features'])}" + ("; requires: " + ", ".join(item["requires"]) if item["requires"] else ""))
                if args.action == "show":
                    print("  source: " + item["source"])
        return 0
    except (CatalogError, OSError, json.JSONDecodeError, subprocess.TimeoutExpired) as error:
        print(f"test-catalog: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
