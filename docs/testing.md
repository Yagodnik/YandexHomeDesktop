# Test discovery and selection

Qt Test and Qt Quick Test own assertions, data rows, and test lifecycle. CTest
owns execution, environments, working directories, and timeouts.
`tests/catalog.json` adds descriptions, feature tags, and build requirements;
`scripts/test_catalog.py` discovers actual runners instead of parsing C++/QML.
The tool uses Python 3.9+ and the standard library only.

## Test suites and reports

Use the Just recipes locally and in CI. The recipes build the selected profile
and invoke CTest with the registered test commands, environments, and timeouts.

| Directory | Recipe | Scope |
| --- | --- | --- |
| `tests/unit/` | `just test-unit` | In-process C++ checks, including model and service integration with local dependencies |
| `tests/ui/` | `just test-ui` | Offscreen QML page and component checks |
| `tests/e2e/` | `just spix=ON test-e2e` | Spix checks against the running desktop executable |
| `tests/process/` | `just test-process` | External executable, local IPC, and SDK checks |

Run `just test` for all tests enabled in the current profile. Use a suite or
case ID to narrow any of the suite recipes, for example `just test-ui HomeUi`
or `just test-unit DeviceControl::EmptyAttributesStillInitializeViewModel`.
Use `just tests-list --layer ui` to browse the catalog by type.

Each run prints the path to an offline HTML report under
`<build-dir>/test-reports/run-*/index.html`. Open it for test descriptions,
status, duration, labels, and links to test output. Reports are written for failing runs too.
Spix reports include the app log. Fixture flows save a PNG at each named
checkpoint after the window image stays stable for 200 ms. The settling wait
is capped at one second for continuously animated content. On failure, Spix
also attempts an immediate screenshot.

With FFmpeg on PATH, fixture flows sample the live window at up to 20 frames
per second so playback includes transitions between checkpoints. Encoding
runs after the test, and the report links to the resulting GIF. Pass
`--record` when GIF generation is required. GitHub Actions uploads the report
directories with their linked files.

### GitHub Actions

The Linux, macOS, and Windows Release workflows run their configured suites and
upload a platform-specific test report. Windows and Linux also run the Spix smoke test.

To run the full Windows Debug suite, open **Actions → Windows Release build →
Run workflow** and enable `full_tests`. To enable it for pull requests, set the
repository Actions variable `CI_FULL_TESTS` to `true` under **Settings → Secrets
and variables → Actions → Variables**. Unset it or set it to `false` to turn off
the extra PR job. The manual checkbox controls manual runs independently.

The Debug job runs unit, QML UI, process, and Spix end-to-end tests, including
fixture flows, using the example OAuth configuration. Download and extract
`windows-full-test-report` from the run's **Artifacts** section and open
`run-*/index.html`. The bundle includes JUnit XML, test logs, and available Spix
screenshots and GIFs. It is retained for seven days and is uploaded even when
tests fail.

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
groups suites into `unit`, `ui`, `e2e`, `process`, and `tooling` layers.
Feature tags and layer names are also CTest labels.

Each suite entry has an `id`, behavior `description`, `features`, `layer`,
runner `kind` (`qt`, `quick`, or `ctest`), and repository-relative `source`.
Qt suites have a `cases` array, with a native function `name` and a description
of the guarantee checked. A case can override `source` when a suite spans
multiple files. Legacy smoke cases without assertions are described as such.
Suite/case `requires` arrays identify actual compile
conditions: `BUILD_DESKTOP_APP`, `BUILD_CLI_APP`, `BUILD_REST_APP`,
`YH_BUILD_REST_ADAPTER`, `YH_ENABLE_SPIX`, and `Debug`. Requirements describe whether the test is
compiled, rather than suppressing a test whose assertions fail.

When adding a test, add its description to the catalog in the same change.
Preserve test names when the checked behavior is unchanged. Moved UI cases use
their new suite prefix, such as `HomeUi::PageStateAndHouseholdClicksReachViewModel`. For a new suite,
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
catalog validation and `TestTooling`. Windows, macOS, and Linux cover
desktop/QML suites. Windows and Linux run a Release Spix smoke test, and the
optional Windows Debug job runs all fixture flows.

## Choose verification for a feature

Identify the changed observable behavior and use the catalog to find its
service/model suites and any actual QML interaction or executable checks.
Add deterministic cases for uncovered behavior. Run the relevant full suites
for verification; use concrete cases to reproduce or investigate a failure.
Changes to shared contracts or test infrastructure usually warrant all tests
in the affected desktop and portable profiles. Report the selected IDs/tags,
configuration, result, and unavailable or skipped coverage.

## Spix flows

Use Spix for end-to-end checks of the running desktop executable over XML-RPC.
See the [build guide](build.md#spix-ui-automation) for the opt-in build and
recording commands. Register flows in `tests/e2e/CMakeLists.txt` and in the
catalog. Keep fixtures credential-free, use existing QML object names when
possible, and wait for an observable result after input. `page_ready` checks
visibility and stack transition state. Use `in_window` for animated items that
can report visible while offscreen. Each Spix test uses the same CTest
resource lock for port 9000.

Use a separate fixture flow for each desktop behavior that should be selectable
on its own. Include inputs a user might make by mistake and check the resulting
UI state. `session.click_burst` queues clicks together without waiting for a
response between clicks. Use `mouseClickWithOffset` or
`mouseClickWithProportion` to click just outside an interactive target.

List the flows with `just tests-list --layer e2e`, run one with
`just spix=ON test-e2e HouseholdPicker`, or run all of them with
`just spix=ON test-e2e --record`.

```python
from spix_harness import WINDOW, run_flow_cli


def run_flow(session):
    devices = f"{WINDOW}/devicesTab"
    session.wait_for(lambda: session.page_ready(devices), "the Devices tab")
    session.checkpoint("Devices tab is ready")


if __name__ == "__main__":
    run_flow_cli("ExampleFlow", run_flow, "Verify the Devices tab", fixture=True)
```

### Native tray placement

`TrayPlacement` opens the running fixture app from its native tray icon on Windows or macOS,
checks that the popup fits inside the icon's display work area, and checks its
alignment with the icon. The report includes an uncropped PNG for every
connected display. If the flow fails, it also attempts to capture the desktop
at the point of failure.

Use a dedicated, unlocked desktop. The test sends a native click and
captures the entire desktop. On macOS, the runner needs Xcode command-line
tools, Accessibility and Screen Recording permissions, and a visible menu
bar with room for the app icon. On Windows, use an interactive runner in the
same desktop session as Explorer, keep the bottom taskbar visible, and pin
the app's notification icon outside the overflow. To pin it, launch the
fixture app from the same build path the test will use, enable tray mode
in Settings, and move the icon to the visible notification area.

Configure a separate Debug build to opt in. On macOS, run the following
commands with the expected values for the display you have configured.

```sh
just build_dir=build-just/native-tray spix=ON configure -DYH_TEST_NATIVE_TRAY=ON
xcrun swift tests/e2e/macos_desktop.swift displays
YH_TRAY_DISPLAY_LAYOUT='[[0,0,1440,900,2]]' \
  just build_dir=build-just/native-tray spix=ON test-e2e TrayPlacement
```

On Windows, use PowerShell with the Qt and MinGW paths configured.

```powershell
just build_dir=build-just/native-tray spix=ON configure '-DCMAKE_CXX_COMPILER=g++.exe' -DYH_TEST_NATIVE_TRAY=ON
just build_dir=build-just/native-tray spix=ON build --target yh_windows_runtime
& ./build-just/native-tray/yh_tray_windows_desktop.exe displays
$env:YH_TRAY_DISPLAY_LAYOUT = '[[0,0,1280,800,1]]'
just build_dir=build-just/native-tray spix=ON test-e2e TrayPlacement
```

Set `YH_TRAY_DISPLAY_LAYOUT` to the expected list of
`[x, y, width, height, scale]` values from the platform's display probe.
Display sizes are in logical units. Windows retains the monitor origins in
physical pixels. The origin of the main display is `[0,0]` on both platforms.
The test fails if the active layout differs.
Repeat on separate configured desktops or change the arrangement in System
Settings between runs. Useful profiles include a compact single display, a
larger single display, and mixed-scale dual displays with the secondary
display left of or above the main one. Swap the main display in a dual setup
to exercise the popup from both physical displays. For example, a secondary
1920×1080 display to the left of a 1440×900 main display could use
`[[0,0,1440,900,2],[-1920,0,1920,1080,1]]`. Each run saves its own
report under the build directory's `test-reports/` directory. The native
tray suite is available on Windows and macOS.

#### Native tray CI

The Windows and macOS build workflows each run two optional native tray jobs
on dedicated self-hosted desktops. Register runners with the standard OS
label and the profile label in the table. Provision CMake, Ninja, Git Bash on
Windows, and the interactive desktop prerequisites above. The jobs install
the pinned Qt kit and build from the example OAuth configuration.
Set each Actions repository variable to the expected layout JSON.

| OS label | Profile label | Layout variable |
| --- | --- | --- |
| `Windows` | `tray-compact` | `TRAY_WINDOWS_COMPACT_LAYOUT` |
| `Windows` | `tray-mixed` | `TRAY_WINDOWS_MIXED_LAYOUT` |
| `macOS` | `tray-compact` | `TRAY_MACOS_COMPACT_LAYOUT` |
| `macOS` | `tray-mixed` | `TRAY_MACOS_MIXED_LAYOUT` |

For `tray-compact`, configure one compact display. For `tray-mixed`, use two
extended displays at different scales, with the secondary left of or above
the main display. Scale the main Windows display above 100% so the
notification icon exercises the DPI conversion. On Windows, pin the icon
for the checkout's `build-just/native-tray/YandexHomeDesktop.exe`.

Keep the dedicated runners restricted to trusted workflows. Enable
`tray_tests` when manually running a Windows or macOS build on `main` or
`dev`, or set the repository Actions variable `CI_NATIVE_TRAY` to `true`
to run the build and tray jobs on pushes to those branches. Pull-request
runs use the hosted build jobs. Each tray job uploads its report and
available full-display screenshots to the same build run.

The desktop logger includes UTC timestamps, severity, process ID, and Qt log
category. Set `QT_LOGGING_RULES` to control categories such as
`yh.devices.commands`. To enable Qt diagnostics for the CLI or REST executable,
set `YH_DIAGNOSTIC_LOG` to a writable file path. They append diagnostics to
that file while preserving their normal stdout and stderr output.
