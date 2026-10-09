# Clean build

The CI builds are `.github/workflows/ci-windows.yml`, `ci-macos.yml`, and `ci-linux.yml`. They run for pull requests targeting `main`, `dev`, or `refactoring`, or when started manually; ordinary branch pushes do not trigger them. Windows and macOS build the desktop app and run CTest; Linux builds and runs the portable tests using the Qt 6.9.1 `linux_gcc_64` kit. CMake fetches pinned Boost.Hana and QtKeychain sources into the build directory. No ignored `libs/` directory or machine-specific CMake file is required.

For a local desktop build, install Qt 6.9 or newer with Quick, QuickTest, NetworkAuth, Widgets, Svg, Qt5Compat, LinguistTools, and Test, plus a C++23 compiler and CMake 3.28 or newer. Configure Qt through the standard CMake prefix path:

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/<kit> -DBUILD_DESKTOP_APP=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Precompiled headers are enabled by default for the main C++ targets. Configure with `-DYH_ENABLE_PCH=OFF` to build without them; Windows CI checks both modes. PCH uses the declared Qt, C++ standard library, and pinned Boost.Hana headers and writes generated files only under the build directory. Shared API model codecs are compiled in `YandexApiAdapter`; custom serializable types still include `serialization/Serialization.h` where their codecs are instantiated.

On Windows, use the compiler kit that matches the Qt binaries and add `--config Release` to build and `-C Release` to CTest if using a multi-config generator. CMake downloads the pinned sources on the first configure. `FETCHCONTENT_SOURCE_DIR_HANA` and `FETCHCONTENT_SOURCE_DIR_QTKEYCHAIN` can point at already downloaded copies when working offline; use the revisions in `CMakeLists.txt`. On Linux, configure with `-DBUILD_DESKTOP_APP=OFF -DBUILD_TESTING=ON` to build the portable tests.

Desktop CTest runs the C++ suite, creates all nine QML pages with local test models, and runs Qt Quick layout and interaction tests. The QML tests use the offscreen platform, software renderer, and bundled font/assets; they need no sign-in or live API access and run in both Windows and macOS CI. No OAuth configuration is included in the QML test executables.

`Authorization` tests session restoration, sign-in, storage failures, cancellation, stale completions, logout ordering, and the QML model using injected stores and flows. It links the Qt Core authentication library and runs in portable builds too, without NetworkAuth, QtKeychain, credentials, a browser, or a callback listener. Desktop `OAuthAdapter` tests configuration validation and lazy setup failures without live OAuth or Keychain access. Windows and macOS CI embed `resources/auth/example.json` and run these checks with the rest of CTest. The pinned dependency versions and clean-build commands above also cover the new targets. See [authorization-architecture.md](authorization-architecture.md) for the dependency and lifetime contracts.

The `Scenarios` test covers loading, execution, refresh, and session reset through a fake `IHomeApi`. Desktop builds also exercise the scenarios page with real mouse clicks; portable builds run the same service and view model checks without Qt Quick.

`Home` checks the shared home snapshot, list models, household selection, filtering, and stale responses after logout. Desktop builds also exercise refresh and household selection through QML. `ApiFixture` checks fixture playback, startup flags, callback lifetimes, and temporary settings. Debug desktop builds additionally test local sign-in and run the executable with `--fake-api --list-devices`. Windows CI builds and tests both Debug and Release with PCH on and off.

The same `Home` target checks room/Favorites collapse, favorite addition order, lexicographic room sorting (including duplicate names and shuffled snapshots), household filtering, persistence using temporary settings files, account switching, and late account responses after logout. Its desktop checks use the already required Qt QuickTest library to wait for layout updates before clicking headings and both room/favorite star buttons through the real QML page. `QmlLayouts` also checks arrow spacing, long-title elision, empty Favorites, animated intermediate states, and reversing an animation before it completes. Windows CI runs these targets in its existing CTest matrix and checks the English translations for the new controls; no credentials or live API requests are needed.

`DeviceControl` characterizes concurrent desktop actions and external device changes through the real device view model, session, and QML-facing models, using a scripted API and controlled clock. It covers polling suppression, action failures, repeated commands, out-of-order reads, and the exact 800 ms boundaries without sleeps or live API access. Its current-behavior contract, including known surprising outcomes, is in [device-control-behavior.md](device-control-behavior.md). `DeviceService` tests explicit-ID commands, independent sessions, structured errors, and callback lifetimes using only Qt Core/Test and `AppServices`. Desktop QML tests also click through a real device capability delegate and exercise load failure/retry. The QML test executables declare their Qt Test and existing `Iot` dependencies in CMake, and the Windows Debug/Release/PCH matrix builds and runs this same path.

`Cli` checks the command and capability input registries, selectors, typed actions, device metadata validation, output/errors, timeout cancellation, and callback lifetimes using `AppCli`, `IotCore`, Qt Core/Test, and scripted APIs. `CapabilityRules` verifies typed validation and device metadata independently of the CLI and GUI, linking only `IotCore` and Qt Core/Test. The existing `All` suite also verifies that QML capability wrappers preserve their payloads and defaults when delegating to the shared core. It runs in portable builds too. Desktop builds produce the console executable `YandexHomeCli` in addition to the app. `CliProcess_YandexHomeCli` and `CliProcess_appYandexHomeDesktop` verify both entry points without live credentials: Debug uses committed fixture data, and Release checks fixture rejection. These targets are built and tested by the Windows Debug/Release/PCH matrix; its Release jobs also install and smoke-test the deployed console CLI. See [cli.md](cli.md) for usage, installation paths, and extension points.

The `Cli` suite also verifies that progress clears before success, failure, and timeout output, and that `--no-progress`, JSON mode, and redirected stderr suppress the indicator. On macOS it verifies native main-queue callback delivery without reading Keychain credentials. Both executable entry points configure this native event loop. The same CLI and process checks run in the Windows Debug/Release/PCH matrix; only the macOS-specific callback check is skipped there.

## Debug fixture API

Configure with `-DCMAKE_BUILD_TYPE=Debug` (or select the Debug configuration in a multi-config build). Start the executable directly to show the normal UI with bundled dummy data:

```sh
YandexHomeDesktop --fake-api
YandexHomeDesktop --fake-api --fake-api-data /path/to/fixture.json
YandexHomeDesktop --fake-api --list-devices
```

On macOS the executable is `<build-dir>/YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop`; on Windows it is `<build-dir>/YandexHomeDesktop.exe`. The flags are recognized before CLI dispatch, so `--fake-api` alone opens the UI. Release builds reject the flags and do not link the fixture API or bundle its data.

Copy `resources/debug/api.json` as the starting point for a custom fixture. `user_info` contains the usual home API response with devices, rooms, households, and scenarios. `account_info` provides a dummy display name, email, and stable `id` for account-specific room/favorite preferences. `device_info` maps device IDs to device-detail API responses. `latency_ms` controls the queued response delay (0–60000 ms). `scenario_errors` maps scenario IDs to execution failure messages; other active scenarios succeed locally. Set `user_info.status` to `error` and supply `message` to exercise loading failures.

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

The root `justfile` provides shortcuts for the same CMake and CTest commands. Install [just 1.51.0](https://github.com/casey/just/releases/tag/1.51.0) and Bash 3.2+ in addition to the build tools above. On Windows, install Git for Windows and put its Bash on `PATH`; the `windows-2022` CI runner already supplies it. CI pins this just version; direct CMake builds remain available. Bash recipes invoke the build tools directly and forward arguments with `"$@"`, preserving paths, device names, and JSON values. Python 3.9+ is needed only for translation checks and updates (`python3` on macOS/Linux, `python` on Windows). Run `just` or `just --list` to see the commands.

```sh
just qt_prefix=/path/to/Qt/6.x/macos configure
just build
just demo                                    # Debug GUI with bundled fixture data
just demo --fake-api-data "/path/to/my fixture.json"
just run                                     # GUI using saved sign-in / real OAuth
just cli --fake-api devices list --json
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
just translations-check
just translations-update                    # Then translate new English messages
```

Set variables before the recipe name. `config` defaults to `Debug`; desktop profiles use `build-just/debug`, `build-just/release`, etc. `desktop=OFF` uses separate `build-just/portable/<configuration>` directories and is the default on Linux. `qt_prefix` falls back to `CMAKE_PREFIX_PATH` and then `QT_ROOT_DIR`. `auth_config` falls back to `YH_AUTH_CONFIG_FILE` and then the credential-free example. For local browser sign-in, run `just auth_config=/absolute/path/to/oauth.json configure`. `pch=ON`, `jobs=4`, and `generator=Ninja` are the defaults; `build_dir=/path/to/build` selects an existing or custom build directory. Multi-config generators are supported through `--config`/`-C`, and the launch recipes search the configuration subdirectory too.

`build` configures only when its directory has no `CMakeCache.txt`. Subsequent builds, launches, and tests preserve configured Qt paths, credentials, and CMake flags. Run `configure` explicitly when changing these settings; append additional CMake options there, for example `just configure -DYH_ENABLE_PCH=OFF`. `build`, `test`, `run`, `cli`, `demo`, and `install` forward extra arguments to their corresponding tools. `demo` requires a Debug desktop profile. `clean` invokes CMake's clean target and retains the build configuration. The Windows Debug/Release/PCH matrix installs pinned just and exercises configuration, builds, tests, formatting, and a credential-free CLI invocation through these recipes.

When switching Qt installations, refresh the CMake cache so cached `Qt6*_DIR` entries do not keep selecting the previous kit:

```sh
just qt_prefix=/path/to/Qt/6.x/macos configure --fresh
just build
```

If a macOS build fails with `ld: framework 'AGL' not found`, check the cached Qt kit and selected Xcode SDK. Older Qt kits can reference AGL even when the selected SDK no longer supplies it. Use a compatible Qt/SDK combination and configure with `--fresh` as above.
