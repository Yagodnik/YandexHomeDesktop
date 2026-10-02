# YandexHomeDesktop

Qt 6 desktop application for Yandex smart home devices. The desktop app is built for Windows and macOS; Linux runs the portable C++ tests only.

## Local build and tests

Use CMake 3.28 or newer, Ninja, Qt 6.9 with NetworkAuth, Qt5Compat, and ShaderTools, and a C++23 compiler. CMake fetches pinned Boost.Hana and QtKeychain sources. QtKeychain is built only for the desktop app.

The app embeds an OAuth JSON resource at build time. Set `AUTH_CONFIG_FILE` to a JSON file with `auth_url`, `access_token_url`, `client_id`, `redirect_base`, `redirect_port`, and `scopes`. Set `client_secret` to an empty string; a real secret cannot remain private inside a distributed app.

```sh
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.1/macos \
  -DAUTH_CONFIG_FILE=/path/to/oauth.json -DBUILD_DESKTOP_APP=ON
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

For portable tests without a desktop app, use `-DBUILD_DESKTOP_APP=OFF`. The OAuth file and QtKeychain are then unnecessary.

## CI and releases

The Linux, Windows, and macOS workflows run on pull requests and pushes to `main` and `dev`. CI builds use a dummy OAuth resource and need no repository secrets. The macOS workflow tests both Apple Silicon and Intel runners.

The release workflow runs when a `vMAJOR.MINOR.PATCH` tag is pushed. Update `project(... VERSION ...)` in `CMakeLists.txt` first, merge that commit into `main`, then tag it. The workflow checks the tag and version, rebuilds and tests Windows and a universal macOS app, and publishes an NSIS installer and DMG to GitHub Releases.

Before creating the first release tag, set the repository secret `RELEASE_OAUTH_CONFIG_JSON` to the complete OAuth JSON object. Its `client_secret` must be empty. The release files are currently unsigned and are likely to show Windows or macOS trust prompts.
