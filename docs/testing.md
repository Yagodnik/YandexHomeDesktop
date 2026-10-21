# Test discovery and selection

Qt Test and Qt Quick Test own assertions, data rows, and test lifecycle. CTest
owns execution, environments, working directories, and timeouts.
`tests/catalog.json` adds descriptions, feature tags, and build requirements;
`scripts/test_catalog.py` discovers actual runners instead of parsing C++/QML.
The tool uses Python 3.9+ and the standard library only.

## Find tests

These commands work before a build is configured:

```sh
just tests-list --suites
just tests-list --feature devices
just tests-show DeviceControl
just tests-show DeviceControl::EmptyAttributesStillInitializeViewModel
just tests-list --json
```

A concrete C++ case ID is `Suite::Function`. A QML case ID includes the QML
TestCase name: `QmlLayouts::PageComposition::test_deviceCommandThroughRealDelegate`.
CTest-only smoke checks use their suite ID, for example `InstalledSdkConsumer`.
Selecting a data-driven function runs all its data rows.

To inspect availability in a configured profile:

```sh
just tests-list --build-dir build-just/debug --config Debug --available
```

Without `--available`, unavailable cases remain visible and are marked. QML
interaction cases in `Home`/`Scenarios` require a desktop build; fixture sign-in
cases additionally require Debug. A listed case can still contain a platform
skip (for example native macOS callback delivery in `Cli`). Listing and
discovery do not constitute a passing test run.

## Run a selection

Use the [build guide](build.md) to configure the desired profile. The run/check
recipes build first and preserve an existing build's configuration:

```sh
just tests-run DeviceControl DeviceService
just tests-run DeviceControl::EmptyAttributesStillInitializeViewModel
just tests-run Serialization::FromJsonSimple QmlLayouts::PageComposition::test_authAction
just tests-run --feature devices
just tests-run --feature auth --feature scenarios
just tests-run --feature devices --dry-run
just tests-run --all
just build_dir=build-rest-release config=Release tests-run RestProcess::JsonConfigurationAndExplicitTokenAvoidSecretStorage
just desktop=OFF tests-run --feature devices
```

Feature tags select complete suites conservatively; repeated tags are a union.
The command prints suites omitted because the selected profile cannot build
them. An explicitly requested unavailable suite/case, unknown ID/tag, empty
selection, or discovery mismatch is an error. Full-suite selection takes
precedence over selected cases from that same suite. Concrete selections are
grouped per suite so cases from one QML runner are not executed concurrently.

The tool copies the selected commands and properties from CTest's JSON
inventory into a temporary CTest file under the build directory. CTest executes
that selection with its original environment, working directory, and timeout.
It prints failure output and returns a nonzero exit code on failure. Temporary
selection files/logs are removed after execution; the console output is the
result of a selected run. Use ordinary CTest for persistent standard logs or
additional CTest options:

```sh
just test -L devices
just test -R '^DeviceControl$'
```

Direct invocation is also available, including for existing builds:

```sh
python3 scripts/test_catalog.py run --build-dir build --config Debug DeviceControl
python3 scripts/test_catalog.py check --build-dir build --config Debug
```

Use `python` on Windows. Supply `--config` for multi-config generators; a
single-config build defaults to the configuration recorded in its cache and
rejects a conflicting override. `--ctest /path/to/ctest` selects a particular
CTest installation.

## Structure and maintain the catalog

The former `All` runner is split into `Serialization`, `Api`, `ApiBoundary`,
`ModelBoundary`, `Capabilities`, and `Models`. Each invokes one Qt test class;
all existing assertions remain in their original source files. The catalog
groups suites into `unit`, `integration`, `ui`, `process`, and `tooling` layers.
Feature tags and layer names are also CTest labels.

Each suite entry has an `id`, behavior `description`, `features`, `layer`,
runner `kind` (`qt`, `quick`, or `ctest`), and repository-relative `source`.
Qt suites have a `cases` array, with a native function `name` and a description
of the guarantee checked. A case can override `source` when a suite spans
multiple files. Legacy smoke cases without assertions are described as such.
Suite/case `requires` arrays identify actual compile
conditions: `BUILD_DESKTOP_APP`, `BUILD_CLI_APP`, `BUILD_REST_APP`,
`YH_BUILD_REST_ADAPTER`, and `Debug`. Requirements describe whether the test is
compiled, rather than suppressing a test whose assertions fail.

When adding a test, add its description to the catalog in the same change.
Preserve case IDs when the checked behavior is unchanged. For a new suite,
register it with `yh_add_test(NAME ... COMMAND ...)`; this reads labels from the
catalog and supplies a default 30-second timeout. Override properties with
`set_tests_properties` when needed. Custom Qt runners must forward `-functions`
and case selectors after consuming their own startup arguments, as
`RestProcessTests` does. Case discovery uses the registered test environment.
It omits Qt `-o` output options so discovery cannot truncate a concurrently
running suite's saved execution log.

```sh
just tests-check
python3 scripts/test_catalog.py check                  # Metadata only; no build
python3 tests/TestCatalogToolTests.py                   # Tool behavior, needs CTest
```

`TestCatalog` runs in normal CTest and reconciles enabled suites and cases with
the configured CTest/Qt inventory. Missing descriptions, duplicate IDs,
uncatalogued cases, stale entries, and mismatched labels fail validation.
Reconfigure after suite/label changes; CMake tracks the catalog as a configure
dependency. `TestTooling` checks exact selection, profile filtering, error
handling, argument/environment preservation, and real CTest timeouts/failures.
Each CI platform runs the full enabled CTest suite once in Release, including
catalog validation and `TestTooling`. Windows/macOS cover desktop/QML suites;
Linux covers portable suites. Debug-only fixture process cases require a local
Debug profile. Use the selection recipes above for focused local verification.

## Choose verification for a feature

Identify the changed observable behavior and use the catalog to find its
service/model suites and any actual QML interaction or executable checks.
Add deterministic cases for uncovered behavior. Run the relevant full suites
for verification; use concrete cases to reproduce or investigate a failure.
Changes to shared contracts or test infrastructure usually warrant all tests
in the affected desktop and portable profiles. Report the selected IDs/tags,
configuration, result, and unavailable or skipped coverage.
