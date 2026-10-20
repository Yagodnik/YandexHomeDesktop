"""Behavioral tests for the test catalog, including execution through real CTest."""

import copy
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import time
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "scripts" / "test_catalog.py"
spec = importlib.util.spec_from_file_location("test_catalog", SCRIPT)
catalog = importlib.util.module_from_spec(spec)
spec.loader.exec_module(catalog)


def suite():
    return {
        "id": "Example", "description": "Example behavior", "features": ["devices"],
        "layer": "unit", "kind": "qt", "source": "tests/TestCatalogToolTests.py",
        "cases": [
            {"name": "First", "description": "First observable behavior"},
            {"name": "DesktopOnly", "description": "Desktop behavior", "requires": ["BUILD_DESKTOP_APP"]},
        ],
    }


def test_record(name="Example", command=None, extra=None):
    props = {"LABELS": ["devices", "unit"], "TIMEOUT": 5}
    props.update(extra or {})
    return {"name": name, "command": command or [sys.executable],
            "properties": [{"name": key, "value": value} for key, value in props.items()]}


class CatalogTests(unittest.TestCase):
    def test_metadata_rejects_duplicate_ids_cases_and_missing_descriptions(self):
        examples = []
        examples.append([suite(), suite()])
        entry = suite()
        entry["cases"].append(copy.deepcopy(entry["cases"][0]))
        examples.append([entry])
        entry = suite()
        del entry["cases"][0]["description"]
        examples.append([entry])
        entry = suite()
        entry["requires"] = ["UNKNOWN_FLAG"]
        examples.append([entry])
        entry = suite()
        entry["cases"][0]["source"] = "../outside-checkout.cpp"
        examples.append([entry])
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / "catalog.json"
            for entries in examples:
                with self.subTest(entries=entries):
                    path.write_text(json.dumps({"version": 1, "suites": entries}))
                    with self.assertRaises(catalog.CatalogError):
                        catalog.load_catalog(path)

    def test_exact_selection_never_broadens_a_misspelled_case_or_feature(self):
        chosen, unavailable = catalog.select([suite()], ["Example::First"], [], False, {})
        self.assertEqual(chosen, {"Example": {"First"}})
        self.assertEqual(unavailable, [])
        for selectors, features in [(["Example::Typo"], []), (["Missing"], []), ([], ["typo"]), ([], [])]:
            with self.subTest(selectors=selectors, features=features):
                with self.assertRaises(catalog.CatalogError):
                    catalog.select([suite()], selectors, features, False, {})

    def test_full_suite_dominates_cases_and_selection_is_deduplicated(self):
        for selectors in (["Example::First", "Example", "Example::First"], ["Example", "Example::First"]):
            chosen, _ = catalog.select([suite()], selectors, [], False, {})
            self.assertEqual(chosen, {"Example": None})

    def test_build_requirements_distinguish_omitted_and_explicit_tests(self):
        entry = suite()
        entry["requires"] = ["BUILD_DESKTOP_APP"]
        portable = suite()
        portable["id"] = "Portable"
        chosen, unavailable = catalog.select([entry, portable], [], ["devices"], False, {})
        self.assertEqual(chosen, {"Portable": None})
        self.assertEqual(unavailable, ["Example"])
        for selector in ("Example", "Portable::DesktopOnly"):
            with self.assertRaises(catalog.CatalogError):
                catalog.select([entry, portable], [selector], [], False, {})
        records = catalog.records([portable], {})
        self.assertEqual([record["available"] for record in records], [True, False])

    def test_empty_feature_selection_is_an_error(self):
        entry = suite()
        entry["requires"] = ["BUILD_DESKTOP_APP"]
        with self.assertRaises(catalog.CatalogError):
            catalog.select([entry], [], ["devices"], False, {})

    def test_discovery_uses_the_registered_arguments_environment_and_directory(self):
        with tempfile.TemporaryDirectory(prefix="catalog discovery ") as temp:
            directory = Path(temp).resolve()
            script = directory / "fake qt.py"
            script.write_text(
                "import os, sys\n"
                "assert sys.argv[1:] == ['app with spaces', '-functions']\n"
                "assert os.getcwd() == os.environ['EXPECTED_DIRECTORY']\n"
                "print('First()')\n"
            )
            record = test_record(command=[sys.executable, str(script), "app with spaces"], extra={
                "ENVIRONMENT": ["EXPECTED_DIRECTORY=" + str(directory)], "WORKING_DIRECTORY": str(directory)})
            self.assertEqual(catalog.discover_cases(record), {"First"})
            catalog.check_inventory([suite()], {"Example": record}, {})
            with self.assertRaisesRegex(catalog.CatalogError, "cases differ"):
                catalog.check_inventory([suite()], {"Example": record}, {"BUILD_DESKTOP_APP": "ON"})
            record["command"] += ["unexpected"]
            with self.assertRaisesRegex(catalog.CatalogError, "discovery failed"):
                catalog.discover_cases(record)

    def test_uncatalogued_ctest_entries_fail_validation(self):
        with self.assertRaisesRegex(catalog.CatalogError, "uncatalogued"):
            catalog.check_inventory([suite()], {"Unexpected": test_record("Unexpected")}, {})

    def test_discovery_preserves_execution_logs(self):
        with tempfile.TemporaryDirectory(prefix="catalog logging ") as temp:
            directory = Path(temp)
            log = directory / "worker output.txt"
            log.write_text("Existing execution output\n")
            script = directory / "fake qt.py"
            script.write_text(
                "import pathlib, sys\n"
                "for index, arg in enumerate(sys.argv):\n"
                "    if arg == '-o' and sys.argv[index + 1] != '-,txt':\n"
                "        pathlib.Path(sys.argv[index + 1].rsplit(',', 1)[0]).write_text('')\n"
                "print('First()')\n"
            )
            record = test_record(command=[sys.executable, str(script),
                                          "-o", "-,txt", "-o", str(log) + ",txt"])
            self.assertEqual(catalog.discover_cases(record), {"First"})
            self.assertEqual(log.read_text(), "Existing execution output\n")

    def test_qt_quick_discovery_accepts_function_names_on_stderr(self):
        record = test_record(command=[sys.executable, "-c", "import sys; print('Page::test_click()', file=sys.stderr)"])
        self.assertEqual(catalog.discover_cases(record), {"Page::test_click"})

    def test_read_profile_rejects_another_checkout_or_wrong_configuration(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp).resolve()
            cache = directory / "CMakeCache.txt"
            cache.write_text(f"CMAKE_HOME_DIRECTORY:INTERNAL={catalog.ROOT}\nBUILD_TESTING:BOOL=ON\nCMAKE_BUILD_TYPE:STRING=Debug\n")
            profile, config = catalog.read_profile(directory, "")
            self.assertEqual(config, "Debug")
            self.assertTrue(catalog.enabled({"requires": ["Debug"]}, profile))
            with self.assertRaises(catalog.CatalogError):
                catalog.read_profile(directory, "Release")
            cache.write_text("CMAKE_HOME_DIRECTORY:INTERNAL=/another/checkout\nBUILD_TESTING:BOOL=ON\n")
            with self.assertRaises(catalog.CatalogError):
                catalog.read_profile(directory, "")

    def test_cli_reports_unknown_ids_with_a_nonzero_exit(self):
        result = subprocess.run([sys.executable, str(SCRIPT), "show", "Missing"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn("Unknown test ID", result.stderr)


class CTestExecutionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.ctest = shutil.which("ctest")
        if cls.ctest is None:
            raise RuntimeError("CTest is required for execution tests")

    def test_selection_preserves_arguments_environment_working_directory_and_failures(self):
        with tempfile.TemporaryDirectory(prefix="catalog execution ") as temp:
            directory = Path(temp).resolve()
            script = directory / "fake tests.py"
            output = directory / "received.json"
            script.write_text(
                "import json, os, pathlib, sys\n"
                "pathlib.Path(os.environ['RESULT_PATH']).write_text(json.dumps({"
                "'args':sys.argv[1:], 'directory':os.getcwd(), 'value':os.environ['DISPLAY_VALUE']}))\n"
                "sys.exit(1 if 'Fail' in sys.argv else 0)\n"
            )
            record = test_record(command=[sys.executable, str(script), "literal $() and spaces", "brackets ]=]"], extra={
                "WORKING_DIRECTORY": str(directory),
                "ENVIRONMENT": ["RESULT_PATH=" + str(output), "DISPLAY_VALUE=a value;with semicolon"],
            })
            tests = {"Example": record}
            self.assertEqual(catalog.run_selection(directory, "", self.ctest, tests, {"Example": {"First"}}, 1), 0)
            received = json.loads(output.read_text())
            self.assertEqual(received, {"args": ["literal $() and spaces", "brackets ]=]", "First"],
                                        "directory": str(directory), "value": "a value;with semicolon"})
            self.assertNotEqual(catalog.run_selection(directory, "", self.ctest, tests, {"Example": {"Fail"}}, 1), 0)
            self.assertEqual(list(directory.glob("test-selection-*")), [])

    def test_ctest_enforces_the_registered_timeout(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp).resolve()
            record = test_record(command=[sys.executable, "-c", "import time; time.sleep(5)"], extra={"TIMEOUT": 0.2})
            started = time.monotonic()
            result = catalog.run_selection(directory, "", self.ctest, {"Example": record}, {"Example": None}, 1)
            self.assertNotEqual(result, 0)
            self.assertLess(time.monotonic() - started, 4)


if __name__ == "__main__":
    unittest.main()
