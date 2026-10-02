# Project guidance

## Reproducible builds

A clean checkout must configure, build, and test in CI using only committed source, declared tools, and pinned external dependencies. Do not make the build depend on a developer's ignored `libs/` directory, an absolute machine path, a temporary script, a stale generated file, or an uncommitted credential. Keep build outputs in the build directory; validate any intentionally committed generated source in CI. If a new dependency or generator is needed, declare its version in CMake or the workflow and update the clean-checkout build instructions in `docs/build.md`.

Use `resources/auth/example.json` for credential-free builds. A real OAuth configuration is supplied with `YH_AUTH_CONFIG_FILE` when needed for a local runnable build; do not commit credentials or include them in CI artifacts.

When changing build steps, resources, translations, or dependencies, make the Windows CI workflow exercise the same path. Run the available checks locally and state clearly which platform or network-dependent steps could not be verified.

## UI and translations

Keep reusable QML visuals in `src/qml/components/` or `src/qml/ui/` and register them in `src/qml/CMakeLists.txt`. Keep behavior in the existing page/controller wiring until a separate logic change is requested. See `docs/qml.md`.

Wrap user-facing QML and C++ strings for translation, preserve placeholders, and keep the English catalog complete. For bundled JSON display values, regenerate `translations/DataStrings.cpp` with `scripts/update-data-translation-markers.py`. See `docs/localization.md`.
