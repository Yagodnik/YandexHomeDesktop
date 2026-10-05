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

The default embedded OAuth JSON is `resources/auth/example.json`. It contains no usable credentials, so the resulting binary is suitable for build and test verification but cannot sign in. To build a runnable app, place your JSON outside Git (for example at ignored `resources/auth/secrets.json`) and configure with `-DYH_AUTH_CONFIG_FILE=/path/to/secrets.json` or `-DAUTH_CONFIG_FILE=/path/to/secrets.json`. This JSON is embedded in the binary, so keep binaries built with real credentials private. The file must supply `auth_url`, `access_token_url`, `client_id`, `client_secret`, `redirect_base`, `redirect_port`, and `scopes`.

Before committing translation changes, run:

```sh
python3 scripts/update-data-translation-markers.py --check
python3 scripts/check-translations.py
```

The `.qm` catalog is generated during the CMake build. It is not an input to a clean checkout.
