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

The `Scenarios` test covers loading, execution, refresh, and session reset through a fake `IHomeApi`. Desktop builds also exercise the scenarios page with real mouse clicks; portable builds run the same service and view model checks without Qt Quick.

`Home` checks the shared home snapshot, list models, household selection, filtering, and stale responses after logout. Desktop builds also exercise refresh and household selection through QML. `ApiFixture` checks fixture playback, startup flags, callback lifetimes, and temporary settings. Debug desktop builds additionally test local sign-in and run the executable with `--fake-api --list-devices`. Windows CI builds and tests both Debug and Release with PCH on and off.

`DeviceControl` characterizes concurrent desktop actions and external device changes through the real controller and QML-facing models, using a scripted API and controlled clock. It covers polling suppression, action failures, repeated commands, out-of-order reads, and the exact 800 ms boundaries without sleeps or live API access. Its current-behavior contract, including known surprising outcomes, is in [device-control-behavior.md](device-control-behavior.md).

## Debug fixture API

Configure with `-DCMAKE_BUILD_TYPE=Debug` (or select the Debug configuration in a multi-config build). Start the executable directly to show the normal UI with bundled dummy data:

```sh
YandexHomeDesktop --fake-api
YandexHomeDesktop --fake-api --fake-api-data /path/to/fixture.json
YandexHomeDesktop --fake-api --list-devices
```

On macOS the executable is `<build-dir>/YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop`; on Windows it is `<build-dir>/YandexHomeDesktop.exe`. The flags are recognized before CLI dispatch, so `--fake-api` alone opens the UI. Release builds reject the flags and do not link the fixture API or bundle its data.

Copy `resources/debug/api.json` as the starting point for a custom fixture. `user_info` contains the usual home API response with devices, rooms, households, and scenarios. `account_info` provides a dummy display name and email. `device_info` maps device IDs to device-detail API responses. `latency_ms` controls the queued response delay (0–60000 ms). `scenario_errors` maps scenario IDs to execution failure messages; other active scenarios succeed locally. Set `user_info.status` to `error` and supply `message` to exercise loading failures.

Capability and property entries use the API fields, including `type`, `retrievable`, and `last_updated`. Omit the arrays or use empty arrays when those attributes are not needed.

The file is read for each request, so editing a custom fixture and refreshing the page shows the updated data. Invalid files produce explicit errors; the app does not fall back to the real API. Fixture mode uses local sign-in, a bundled avatar, and in-memory UI settings. It does not access OAuth credentials, keychain tokens, or the HTTP transport. Device-detail browsing works; device actions deliberately return a "not simulated" error until a device simulation is added.

The default embedded OAuth JSON is `resources/auth/example.json`. It contains no usable credentials, so the resulting binary is suitable for build and test verification but cannot sign in. To build a runnable app, place your JSON outside Git (for example at ignored `resources/auth/secrets.json`) and configure with `-DYH_AUTH_CONFIG_FILE=/path/to/secrets.json` or `-DAUTH_CONFIG_FILE=/path/to/secrets.json`. This JSON is embedded in the binary, so keep binaries built with real credentials private. The file must supply `auth_url`, `access_token_url`, `client_id`, `client_secret`, `redirect_base`, `redirect_port`, and `scopes`.

Before committing translation changes, run:

```sh
python3 scripts/update-data-translation-markers.py --check
python3 scripts/check-translations.py
```

The `.qm` catalog is generated during the CMake build. It is not an input to a clean checkout.
