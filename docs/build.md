# Clean build

The supported CI build is `.github/workflows/build-windows.yml`. It runs on Windows 2022 with MSVC 2022 and Qt 6.9.1, configures from a clean checkout, builds, and runs CTest. The workflow installs Qt modules, while CMake fetches fixed Boost.Hana and QtKeychain revisions into the build directory. No ignored `libs/` directory or machine-specific CMake file is required.

For a local build, install Qt 6.8 or newer with Quick, NetworkAuth, ShaderTools, LinguistTools, and Test, plus a C++23 compiler and CMake 3.18 or newer. The macOS build was verified with Qt 6.10.2; older Qt kits may not link with newer Xcode SDKs. Configure Qt through the standard CMake prefix path:

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x/<kit>
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

On Windows, use the MSVC kit that matches the Qt binaries and add `--config Release` to build and `-C Release` to CTest. CMake downloads the pinned sources on the first configure. `FETCHCONTENT_SOURCE_DIR_BOOST_HANA` and `FETCHCONTENT_SOURCE_DIR_QTKEYCHAIN` can point at already downloaded copies when working offline; use the revisions in `CMakeLists.txt`.

The default embedded OAuth JSON is `resources/auth/example.json`. It contains no usable credentials, so the resulting binary is suitable for build and test verification but cannot sign in. To build a runnable app, place your JSON outside Git (for example at ignored `resources/auth/secrets.json`) and configure with `-DYH_AUTH_CONFIG_FILE=/path/to/secrets.json`. This JSON is embedded in the binary, so keep binaries built with real credentials private. The file must supply `auth_url`, `access_token_url`, `client_id`, `client_secret`, `redirect_base`, `redirect_port`, and `scopes`.

Before committing translation changes, run:

```sh
python3 scripts/update-data-translation-markers.py --check
python3 scripts/check-translations.py
```

The compiled shader and `.qm` catalog are generated during the CMake build. They are not inputs to a clean checkout.
