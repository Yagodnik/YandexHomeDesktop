# Clean build

The CI builds are `.github/workflows/ci-windows.yml`, `ci-macos.yml`, and `ci-linux.yml`. They run for pull requests targeting `main`, `dev`, or `refactoring`, or when started manually; ordinary branch pushes do not trigger them. Windows and macOS build the desktop app and run CTest; Linux builds the headless CLI/REST applications and runs the portable tests using the Qt 6.9.1 `linux_gcc_64` kit. CMake fetches pinned Boost.Hana, QtKeychain, and (for the shortcut worker) QHotkey sources into the build directory. No ignored `libs/` directory or machine-specific CMake file is required.

For a local desktop build, install Qt 6.9 or newer with HttpServer, WebSockets, Quick, QuickTest, NetworkAuth, Widgets, Svg, Qt5Compat, LinguistTools, and Test, plus a C++23 compiler and CMake 3.28 or newer. Configure Qt through the standard CMake prefix path:

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/<kit> -DBUILD_DESKTOP_APP=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Precompiled headers are enabled by default for the main C++ targets. Configure with `-DYH_ENABLE_PCH=OFF` to build without them; Windows CI checks both modes. PCH uses the declared Qt, C++ standard library, and pinned Boost.Hana headers and writes generated files only under the build directory. Shared API model codecs are compiled in `YandexApiAdapter`; custom serializable types still include `serialization/Serialization.h` where their codecs are instantiated.

On Windows, use the compiler kit that matches the Qt binaries and add `--config Release` to build and `-C Release` to CTest if using a multi-config generator. CMake downloads the pinned sources on the first configure. `FETCHCONTENT_SOURCE_DIR_HANA` and `FETCHCONTENT_SOURCE_DIR_QTKEYCHAIN` (and `FETCHCONTENT_SOURCE_DIR_QHOTKEY` for desktop builds) can point at already downloaded copies when working offline; use the revisions in `CMakeLists.txt`. All test builds require Python 3.9+ for catalog validation and tooling tests (CI pins Python 3.13). Portable tests also require the Qt HttpServer and WebSockets modules (the CI kit is pinned to Qt 6.9.1). On Linux, install `libsecret-1-dev` for QtKeychain and configure with `-DBUILD_DESKTOP_APP=OFF -DBUILD_TESTING=ON`. This builds CLI, REST, and the portable tests. Headless applications require Qt Core, Network, and LinguistTools; REST also requires HttpServer/WebSockets. Qt Gui is needed only for the desktop or the existing presentation-model tests.

Desktop CTest runs the C++ suite, creates all nine QML pages with local test models, and runs Qt Quick layout and interaction tests. The QML tests use the offscreen platform, software renderer, and bundled font/assets; they need no sign-in or live API access and run in both Windows and macOS CI. No OAuth configuration is included in the QML test executables.

The same `QmlLayouts` runner checks saved capability keybinds: key capture,
cancel/clear, page recreation, cross-device conflicts, account switching, no
device action dispatch, attachment spacing, and the settings visibility toggle.
`KeybindSettings` checks disk persistence, stable action identities, normalized
combinations, duplicate prevention, invalid data, write failures, and demo
isolation. `ApiFixture` checks persistence/reset and fixture isolation for the
visibility preference. These run in the existing Windows CTest matrix;
saved binding storage uses the existing dependencies.
The same QML suite checks independent range decrease/increase assignments and
per-color, temperature, scene, and mode assignments through the real controls,
including normal pointer actions, tab changes, dropdown reopening, narrow
windows, both themes, and hiding the attachments.
It also verifies animated editor opening/closing and reopening after cancellation.
Scenario bindings use the same editor through the real scenarios page. Tests cover
stable IDs, device/scenario conflicts, inactive editing, normal play actions,
account changes, and visibility in both themes. Settings tests check mixed binding
backups and migration from version-1 device-only documents; shortcut runtime and
process tests verify fresh scenario validation, cancellation, and worker execution.
The QML suite has a 90-second timeout to include these real dialog transitions
on slower CI runners.

`SettingsTransfer` verifies current-account settings backup/restore, full-file
validation, duplicate detection, confirmation/cancellation, account changes,
atomic file export, storage failures, and immediate home preference updates.
`QmlLayouts::SettingsTransfer` exercises the real file-dialog wiring with local
temporary JSON files and checks the card and confirmation at narrow widths in
both themes. Native OS pickers are replaced by Qt's file picker in automated
tests. Windows CI runs these suites and concrete selections in every existing
Debug/Release/PCH job. The pickers use Qt Quick Dialogs from the existing desktop
Qt installation, without additional external dependencies.

Settings use the serialization engine's `TryFrom<T>` for checked decoding:
declared fields are required, scalar JSON types and numeric ranges are validated,
and nested structs/lists are checked recursively. Unknown fields remain ignored,
and the existing permissive `From<T>` behavior is unchanged. The `Serialization`
suite checks missing/wrong types, fractional/overflowing integers, nesting, and
64-bit boundaries. This also removes repeated JSON field schemas from keybind
settings validation.

`Authorization` tests session restoration, sign-in, storage failures, cancellation, stale completions, logout ordering, and the QML model using injected stores and flows. It links the Qt Core authentication library and runs in portable builds too, without NetworkAuth, QtKeychain, credentials, a browser, or a callback listener. Desktop `OAuthAdapter` tests configuration validation and lazy setup failures without live OAuth or Keychain access. Windows and macOS CI embed `resources/auth/example.json` and run these checks with the rest of CTest. The pinned dependency versions and clean-build commands above also cover the new targets. See [authorization-architecture.md](authorization-architecture.md) for the dependency and lifetime contracts.

The `Scenarios` test covers loading, execution, refresh, and session reset through a fake `IHomeApi`. Desktop builds also exercise the scenarios page with real mouse clicks; portable builds run the same service and view model checks without Qt Quick.

`Home` checks the shared home snapshot, list models, household selection, filtering, and stale responses after logout. Desktop builds also exercise refresh and household selection through QML. `ApiFixture` checks fixture playback, startup flags, callback lifetimes, and temporary settings. Debug builds additionally test fixture commands through `YandexHomeCli --fake-api --list-devices`. Desktop builds also check local fixture sign-in. Windows CI builds and tests both Debug and Release with PCH on and off.

The same `Home` target checks room/Favorites collapse, favorite addition order, lexicographic room sorting (including duplicate names and shuffled snapshots), household filtering, persistence using temporary settings files, account switching, and late account responses after logout. Its desktop checks use the already required Qt QuickTest library to wait for layout updates before clicking headings and both room/favorite star buttons through the real QML page. `QmlLayouts` also checks arrow spacing, long-title elision, empty Favorites, animated intermediate states, and reversing an animation before it completes. Windows CI runs these targets in its existing CTest matrix and checks the English translations for the new controls; no credentials or live API requests are needed.

`DeviceControl` characterizes concurrent desktop actions and external device changes through the real device view model, session, and QML-facing models, using a scripted API and controlled clock. It covers polling suppression, action failures, repeated commands, out-of-order reads, and the exact 800 ms boundaries without sleeps or live API access. Its current-behavior contract, including known surprising outcomes, is in [device-control-behavior.md](device-control-behavior.md). `DeviceService` tests explicit-ID commands, independent sessions, structured errors, and callback lifetimes using only Qt Core/Test and `AppServices`. Desktop QML tests also click through a real device capability delegate and exercise load failure/retry. The QML test executables declare their Qt Test and existing `Iot` dependencies in CMake, and the Windows Debug/Release/PCH matrix builds and runs this same path.

`Cli` checks the command and capability input registries, selectors, typed actions, device metadata validation, output/errors, timeout cancellation, and callback lifetimes using `AppCli`, `IotCore`, Qt Core/Test, and scripted APIs. `CapabilityRules` verifies typed validation and device metadata independently of the CLI and GUI, linking only `IotCore` and Qt Core/Test. The `Capabilities` suite also verifies that QML capability wrappers preserve their payloads and defaults when delegating to the shared core. It runs in portable builds too. The `YandexHomeCli` executable builds independently of the desktop. `CliProcess_YandexHomeCli` verifies its entry point without live credentials: Debug uses committed fixture data, and Release checks fixture rejection. These targets are built and tested by the Windows Debug/Release/PCH matrix; its Release jobs also install and smoke-test the deployed console CLI. See [cli.md](cli.md) for usage, installation paths, and extension points.

The `Cli` suite also verifies that progress clears before success, failure, and timeout output, and that `--no-progress`, JSON mode, and redirected stderr suppress the indicator. On macOS it verifies native main-queue callback delivery without reading Keychain credentials. Both executable entry points configure this native event loop. The same CLI and process checks run in the Windows Debug/Release/PCH matrix; only the macOS-specific callback check is skipped there.

## Debug fixture API

Configure with `-DCMAKE_BUILD_TYPE=Debug` (or select the Debug configuration in a multi-config build). Start the executable directly to show the normal UI with bundled dummy data:

```sh
YandexHomeDesktop --fake-api
YandexHomeDesktop --fake-api --fake-api-data /path/to/fixture.json
YandexHomeCli --fake-api --list-devices
```

On macOS the executable is `<build-dir>/YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop`; on Windows it is `<build-dir>/YandexHomeDesktop.exe`. Each executable accepts fixture flags: the GUI opens its UI, the CLI executes a command, and REST serves in the foreground. Release builds reject the flags and do not link the fixture API or bundle its data.

Copy `resources/debug/api.json` as the starting point for a custom fixture. `user_info` contains the usual home API response with devices, rooms, households, and scenarios. `account_info` provides a dummy display name, email, and stable `id` for account-specific room/favorite preferences. `device_info` maps device IDs to device-detail API responses. `latency_ms` controls the queued response delay (0–60000 ms). `scenario_errors` maps scenario IDs to execution failure messages; other active scenarios succeed locally. Set `user_info.status` to `error` and supply `message` to exercise loading failures.

The bundled demo includes eight devices, each with several controls and sensor
readings. Home-list attributes and device-detail responses contain the same data.

| Device | Control examples |
| --- | --- |
| Desk lamp | On/off, brightness slider, RGB colors, white temperatures, scenes, backlight toggle |
| Reading socket | On/off, backlight and child-lock toggles, voltage/current/power readings |
| Cabin lamp | On/off, brightness, white temperatures without RGB colors |
| Climate unit | On/off, fractional temperature slider, thermostat and fan-speed modes, ionization toggle, temperature/humidity readings |
| Living room speaker | On/off, relative volume (+/−), mute toggle, input-source modes |
| Color strip (HSV) | On/off, brightness, HSV colors and scenes without white temperatures |
| Scene light | On/off, scene-only color control, pause toggle |
| Camera (unsupported video) | On/off, backlight, unsupported video-stream capability, battery and motion readings |

`ApiFixture::DemoDevicesCoverEveryControlVariant` checks fixture coverage and
consistency. `QmlLayouts::DemoDevices::test_demoDevicePages` bundles the same JSON
into its test executable and opens every device at 320/350 px in both themes,
including mode lists, scene tabs, unsupported content, and scrolling to readings.
These run in the existing Windows Debug/Release CTest matrix; the regular app
continues to bundle demo data only in Debug builds.

Capability and property entries use the API fields, including `type`, `retrievable`, and `last_updated`. Omit the arrays or use empty arrays when those attributes are not needed.

The file is read for each request, so editing a custom fixture and refreshing the page shows the updated data. Invalid files produce explicit errors; the app does not fall back to the real API. Fixture mode uses local sign-in, a bundled avatar, and in-memory UI settings. It does not access OAuth credentials, keychain tokens, or the HTTP transport. Device-detail browsing works; device actions deliberately return a "not simulated" error until a device simulation is added.

The default embedded OAuth JSON is `resources/auth/example.json`. It contains no usable credentials, so the resulting binary is suitable for build and test verification but cannot sign in. To build a runnable app, place your JSON outside Git (for example at ignored `resources/auth/secrets.json`) and configure with `-DYH_AUTH_CONFIG_FILE=/path/to/secrets.json` or `-DAUTH_CONFIG_FILE=/path/to/secrets.json`. This JSON is embedded in the binary, so keep binaries built with real credentials private. The file must supply `auth_url`, `access_token_url`, `client_id`, `client_secret`, `redirect_base`, `redirect_port`, and `scopes`. Authorization and token URLs must use HTTPS. `redirect_base` is an HTTP loopback URL (`127.0.0.1`, `localhost`, or `::1`), optionally with a callback path; put the port in `redirect_port` (1–65535). Public clients use an empty `client_secret`. Configuration is validated only when browser sign-in starts; saved-token restoration and the console CLI do not load it or bind a callback port.

Before committing translation changes, run:

```sh
python3 scripts/update-data-translation-markers.py --check
python3 scripts/check-translations.py
```

The `.qm` catalog is generated during the CMake build. It is not an input to a clean checkout.

## Just commands

For described test discovery and feature/suite/case selection, see
[testing.md](testing.md). `just tests-list` works without a build;
`just tests-run --feature devices` builds and runs the selected suites.

The root `justfile` provides shortcuts for the same CMake and CTest commands. Install [just 1.51.0](https://github.com/casey/just/releases/tag/1.51.0) and Bash 3.2+ in addition to the build tools above. On Windows, install Git for Windows and put its Bash on `PATH`; the `windows-2022` CI runner already supplies it. CI pins this just version; direct CMake builds remain available. Bash recipes invoke the build tools directly and forward arguments with `"$@"`, preserving paths, device names, and JSON values. Python 3.9+ is needed for translation checks, updates, and the desktop REST example tests (`python3` on macOS/Linux, `python` on Windows). Run `just` or `just --list` to see the commands.

```sh
just qt_prefix=/path/to/Qt/6.x/macos configure
just build
just demo                                    # Debug GUI with bundled fixture data
just demo --fake-api-data "/path/to/my fixture.json"
just run                                     # GUI using saved sign-in / real OAuth
just cli --fake-api devices list --json
just rest --fake-api                         # Foreground REST worker
just cli --fake-api --enable-rest            # Detached REST worker
just cli --fake-api --disable-rest
just cli --fake-api devices show --name "Desk lamp" --json
just test -R 'Authorization|OAuthAdapter'
just check                                   # Translations, build, and all tests
just config=Release qt_prefix=/path/to/Qt/6.x/macos configure
just config=Release build
just config=Release run
just config=Release install                  # Install under the profile's stage/
just pch=OFF configure                       # Explicitly update the current cache
just build --target OAuthAdapterTests
just desktop=OFF config=Release qt_prefix=/path/to/Qt/6.x/kit configure
just desktop=OFF config=Release test
just desktop=OFF rest_app=OFF configure -DBUILD_TESTING=OFF
just desktop=OFF rest_app=OFF build-cli
just desktop=OFF cli_app=OFF configure -DBUILD_TESTING=OFF
just desktop=OFF cli_app=OFF build-rest
just translations-check
just translations-update                    # Then translate new English messages
```

Set variables before the recipe name. `config` defaults to `Debug`; desktop profiles use `build-just/debug`, `build-just/release`, etc. `desktop=OFF` uses separate `build-just/portable/<configuration>` directories and is the default on Linux. `qt_prefix` falls back to `CMAKE_PREFIX_PATH` and then `QT_ROOT_DIR`. `auth_config` falls back to `YH_AUTH_CONFIG_FILE` and then the credential-free example. For local browser sign-in, run `just auth_config=/absolute/path/to/oauth.json configure`. `cli_app=ON`, `rest_app=ON`, `pch=ON`, `jobs=4`, and `generator=Ninja` are the defaults; `build_dir=/path/to/build` selects an existing or custom build directory. Multi-config generators are supported through `--config`/`-C`, and the launch recipes search the configuration subdirectory too.

`build` configures only when its directory has no `CMakeCache.txt`. Subsequent builds, launches, and tests preserve configured Qt paths, credentials, and CMake flags. Run `configure` explicitly when changing these settings; append additional CMake options there, for example `just configure -DYH_ENABLE_PCH=OFF`. `build`, `test`, `run`, `cli`, `rest`, `demo`, and `install` forward extra arguments to their corresponding tools. `demo` requires a Debug desktop profile. `clean` invokes CMake's clean target and retains the build configuration. The Windows Debug/Release/PCH matrix installs pinned just and exercises configuration, builds, tests, formatting, and a credential-free CLI invocation through these recipes.

When switching Qt installations, refresh the CMake cache so cached `Qt6*_DIR` entries do not keep selecting the previous kit:

```sh
just qt_prefix=/path/to/Qt/6.x/macos configure --fresh
just build
```

If a macOS build fails with `ld: framework 'AGL' not found`, check the cached Qt kit and selected Xcode SDK. Older Qt kits can reference AGL even when the selected SDK no longer supplies it. Use a compatible Qt/SDK combination and configure with `--fresh` as above.

`build-gui`, `build-cli`, and `build-rest` build individual executables and their dependencies. `run`/`demo`, `cli`, and `rest` use those targets, respectively. A CLI or GUI target also builds its REST worker when `rest_app=ON`. When changing executable options in an existing profile, run `configure` explicitly.

The shared `WorkerControl` target uses only the already declared Qt Core and
Network modules. It is built and installed with the existing SDK targets and
requires no new dependency or generated source. `WorkerControl` tests exercise
independent controller processes, startup/stop races, crash recovery, malformed
messages, callback lifetime, and workers that do not exit. Windows runs these
checks in every Debug/Release/PCH job and repeats the race suite five times;
portable builds run the same Qt Core/Network suite. See
[worker-control.md](worker-control.md) for its ownership contract.

The REST adapter uses the declared Qt HTTP Server module and its WebSockets dependency. Windows, macOS, Linux, and release workflows install these modules with their pinned Qt 6.9.1 kit. `Rest` tests run in portable builds; CTest runs the CLI/REST process integration test and the settings toggle interaction test. See [rest.md](rest.md) for the background controls and credential-free Python example.

Runtime REST profiles need no additional dependency or embedded credential. The
committed `resources/rest/example.json` contains only a host and port; pass a private
file with `--rest-config` for a supplied OAuth token. The existing Windows
Debug/Release/PCH CTest matrix exercises JSON validation, configured listeners,
and synthetic-token worker startup without secret storage or live upstream calls.

`resources/rest/openapi.json` is committed source, bundled by the `AppRest`
CMake resource target and installed under `share/YandexHome/rest/`. It needs no
generator or runtime schema dependency. `Rest` checks resource/source equality,
router coverage, internal references, and the documented action examples; the
Windows matrix and portable CTest runs exercise the same resource path.
For optional full OpenAPI validation while editing the document, use a pinned
authoring tool (separate from the build):

```sh
uvx --from openapi-spec-validator==0.7.2 openapi-spec-validator resources/rest/openapi.json
```

## Release packages

The packaging commands configure a separate Release profile, check translations,
build all applications and tests, run CTest, create a native package with CPack,
and verify its deployed executables. Run each command on its target OS:

```sh
just package                                  # Select the current OS
just package-windows                          # Windows NSIS .exe installer
just package-macos                            # macOS drag-to-Applications .dmg
just package-linux                            # Linux CLI/REST .tar.gz archive
just package-macos '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
just qt_prefix=/path/to/Qt/kit package
just package_build_dir='build-package/custom profile' package
```

`package_build_dir` defaults to `build-package/<os>`; its `dist/` directory
contains `YandexHomeDesktop-<version>-<platform>-<architecture>` with the native
extension. Version comes from the CMake project. macOS uses the configured
architecture, or `universal` for `arm64;x86_64`. Packaging always selects Release,
CLI, and REST, plus the desktop and shortcut worker on Windows/macOS. Extra
arguments go to CMake configuration; `qt_prefix`, `auth_config`, `pch`, `jobs`,
and `generator` work as with other recipes. An explicit `package_build_dir` can
reuse a Release build; the normal Debug profile is untouched.

Windows packaging requires [NSIS 3.12](https://nsis.sourceforge.io/Docs/AppendixF.html)
in addition to the normal matching Qt/compiler kit. CI installs the pinned
Chocolatey package with `choco install nsis --version=3.12.0 --no-progress -y`.
macOS uses CPack's DragNDrop generator and Apple's `hdiutil`; Linux uses CPack's
TGZ generator. No `create-dmg` or extra archive dependency is needed. CPack and
the verification scripts write only inside the package build directory.

By default, desktop packages embed `resources/auth/example.json` and cannot
sign in. Supply `YH_AUTH_CONFIG_FILE` or `auth_config` for a private runnable
build, following the OAuth guidance above. Tag releases use the existing
validated `RELEASE_OAUTH_CONFIG_JSON` public-client configuration. No credentials
are needed for package verification.

The DMG contains the deployed app bundle, CLI, REST, shortcut worker, and their
runtime dependencies, with an Applications link. It is ad-hoc signed by the
existing Qt deployment step; Developer ID signing and notarization are not
configured. Windows includes the installed applications, SDK, and deployed Qt
and compiler runtime. Linux includes CLI/REST, project shared libraries, and
SDK, with a [runtime requirements README](linux-package.md). It requires a
matching shared Qt runtime and compatible system libraries; it is not a
self-contained Linux desktop application. Debian packaging is deferred.

Verification mounts the finished DMG or extracts the finished tar archive and
runs console help without credentials. macOS also checks every application and
QtCore for the requested architectures. Windows checks CPack's deployed payload;
Windows CI additionally silently installs the generated installer into a
temporary directory and verifies the installed executables and runtimes.
The Windows Release/PCH-ON job, both macOS architecture jobs, and Linux CI
exercise these same `just` commands with example OAuth data. The manual Linux
build workflow uploads the `.tar.gz`. Tag releases build and verify all three
formats before publishing them together.

For direct CMake usage, configure with `-DYH_ENABLE_PACKAGING=ON` and the same
application options, then build/test and run:

```sh
cpack --config <build-dir>/CPackConfig.cmake -C Release
cmake -P <build-dir>/CheckPackage.cmake
```

## C++ formatting and analysis

`.clang-format` uses LLVM style with two-space indentation, 100-column wrapping,
and expanded function bodies. `.clang-tidy` enables the Clang analyzer and two
focused bug checks. Both tools are pinned to 21.1.6 in
`requirements-clang-tools.txt`.

Install the tools in a repository-local virtual environment (use `python` and
`build-clang-tools/Scripts/python.exe` on Windows):

```sh
python3 -m venv build-clang-tools
build-clang-tools/bin/python -m pip install --only-binary=:all: -r requirements-clang-tools.txt
just format
just format-check
just format src/cli src/rest
```

The commands find this virtual environment automatically, or use the pinned
versions on `PATH`. By default, formatting and analysis cover `src/app/`, where
the GUI, CLI, and REST executables are assembled. Supply other source files or
folders to expand the scope. Build outputs and external dependencies are outside
this default scope. `just check` includes the formatting check.

For analysis, configure a Ninja profile with PCH disabled, then build and run
clang-tidy using its generated `compile_commands.json`:

```sh
just pch=OFF configure
just tidy
just tidy src/services
```

`just tidy` builds first so generated Qt headers are available. It checks only
selected sources compiled in that profile, without applying fixes. The runner
uses the configured GCC or Clang compiler's system header paths. Windows CI
checks formatting and validates the clang-tidy configuration in every build job,
then runs analysis in the Debug/PCH-OFF job. Linux CI runs analysis on the
headless startup sources.

## Separate executables and shared SDK

Executable startup code is grouped by application under `src/app/`:

- `gui/`: `GuiMain.cpp`, `GuiApp`, and the GUI's `AppContext`.
- `cli/`: `CliMain.cpp`, `CliApp`, and `CliApplication` set up the console runner.
- `rest/`: `RestMain.cpp`, `RestApp`, `RestCommand`, and `RestConfiguration` set up
  the headless HTTP worker and its controls.
- `common/`: startup options and translations.

Reusable CLI commands remain in `src/cli/`, and the HTTP adapter remains in
`src/rest/`. Both libraries consume the existing shared services.

`BUILD_DESKTOP_APP`, `BUILD_CLI_APP`, and `BUILD_REST_APP` select the GUI, CLI,
and REST worker independently. CLI and REST default to ON on all platforms;
the GUI defaults to ON on Windows/macOS. `YH_BUILD_REST_ADAPTER` controls the
reusable HTTP library and defaults to `BUILD_REST_APP`. Disable it for a
CLI-only build that does not have Qt HttpServer installed. The GUI settings
can control a separately installed sibling worker even when it is not built
in this checkout.

```sh
cmake -S . -B build-cli -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/kit \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_DESKTOP_APP=OFF -DBUILD_REST_APP=OFF \
  -DYH_BUILD_REST_ADAPTER=OFF -DBUILD_TESTING=OFF
cmake --build build-cli --target YandexHomeCli

cmake -S . -B build-rest -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/kit \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_DESKTOP_APP=OFF -DBUILD_CLI_APP=OFF -DBUILD_TESTING=OFF
cmake --build build-rest --target YandexHomeRest
```

The existing reusable targets are shared libraries with exported symbols.
`ApiContracts` remains an interface target; GUI models, QML, platform code,
and executable bootstrap code stay private. `AppRuntime` contains settings
and logging; `Utils` contains visual helpers. `YandexAuth` supplies saved-token
access, while `YandexOAuth` supplies the optional browser flow. Headless
executables do not link Qt Gui, Quick, Widgets, or NetworkAuth.

`cmake --install` installs libraries, public headers, the pinned Hana headers,
and a relocatable `YandexHome` CMake package. `--component SDK` installs only
the reusable SDK. `InstalledSdkConsumer` stages that component, configures and
builds `examples/sdk` as an independent project, then runs it without network
or credentials. Windows CI runs it in every Debug/Release/PCH job and also
builds CLI-only and REST-only profiles. See [sdk.md](sdk.md) for consumption
and compatibility requirements.

Windows installation deploys Qt dependencies for all three executables. macOS
installation deploys the CLI and REST worker inside the app bundle, with their
shared libraries and Qt frameworks; it also installs console executables in
`bin/` and SDK libraries in `lib/`. Standalone headless/SDK installs on macOS
and Linux require an installed matching Qt runtime. The bundle installer
includes everything needed by its three applications.

## Shortcut worker

Desktop builds also build and package `YandexHomeShortcuts`. It uses Qt Core/Gui
and QHotkey 1.5.0, pinned to commit
`34330d6ff5d2ca111c376f6d7da66be9d1817430` in `cmake/QHotkey.cmake`;
macOS builds require the Xcode Objective-C++ compiler and AppKit/Carbon frameworks.
CMake checks the pinned macOS source hash and generates a patched copy inside the
build directory: Carbon registration is exclusive and its event handlers install
once. Downloaded sources remain unchanged. `NativeShortcuts` checks cross-process
conflicts and release against that compiled code in macOS CI.
QHotkey is static, its examples/install rules are disabled, and its BSD license
is included under `share/YandexHome/licenses/QHotkey` in the installed package.
`BUILD_SHORTCUTS_APP` defaults to `BUILD_DESKTOP_APP` and supports Windows/macOS.
Set it OFF for a desktop without native shortcuts, or ON with the desktop OFF
for a standalone worker. Portable CLI/REST builds keep it OFF and fetch no QHotkey.
The reusable `AppShortcuts` SDK library requires Qt Core/Network only.

Run `just tests-run --feature shortcuts` for portable runtime tests, real detached
fixture-worker integration, native registration conflicts/release, and QML editor/
settings interactions. Release runs reject fixture flags and skip fixture process
scenarios. Native checks use a rarely used modifier/F19 combination briefly and
skip if the current desktop cannot register it; they do not inject keyboard input.
Windows CI builds/deploys the worker, tests it in every Debug/Release/PCH job,
and checks installed help. See [shortcuts.md](shortcuts.md) for runtime behavior.
