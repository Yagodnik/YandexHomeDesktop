# Project guidance

YandexHomeDesktop is a C++23 and Qt 6 desktop client for Yandex smart home. It uses QML for the interface and targets macOS and Windows.

## Code layout

- `src/api` handles Yandex API requests and response types.
- `src/auth` handles OAuth and stored credentials.
- `src/models` exposes application data to QML.
- `src/iot` contains device capabilities and properties.
- `src/qml` contains pages, components, and controls.
- `tests` contains Qt Test suites.

## Working on the project

- Make focused changes and preserve existing behavior unless the task calls for a change.
- Keep API, authentication, model, and UI responsibilities in their respective directories.
- When adding a C++ or QML file, update the relevant `CMakeLists.txt` and resource declarations.
- Preserve QML-facing type names, properties, and signals when refactoring their implementations.
- Never commit credentials or tokens, or include them in logs and test output.
- Add or update deterministic Qt tests for behavior you change; avoid live API calls in unit tests.

## Verification

- Configure with `cmake -S . -B build -G Ninja`.
- Build with `cmake --build build`.
- Run the test executable at `build/tests/AppTests`.
- Report what was verified and any setup blockers.
