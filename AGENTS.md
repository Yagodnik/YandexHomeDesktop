# Project guidance

## Project skills

Read the relevant repository skill before implementing or reviewing work in these areas:

- [yh-qml-composition](.agents/skills/yh-qml-composition/SKILL.md): QML pages, reusable visuals, property/signal contracts, and UI verification.
- [yh-application-architecture](.agents/skills/yh-application-architecture/SKILL.md): C++ view models, list models, services, API boundaries, and asynchronous lifecycle behavior.

Use both when a feature crosses the QML/C++ boundary. These skills describe the intended architecture; existing compatibility wiring is migrated only within the requested scope.

## Reproducible builds

A clean checkout must configure, build, and test in CI using only committed source, declared tools, and pinned external dependencies. Do not make the build depend on a developer's ignored `libs/` directory, an absolute machine path, a temporary script, a stale generated file, or an uncommitted credential. Keep build outputs in the build directory; validate any intentionally committed generated source in CI. If a new dependency or generator is needed, declare its version in CMake or the workflow and update the clean-checkout build instructions in `docs/build.md`.

Use `resources/auth/example.json` for credential-free builds. A real OAuth configuration is supplied with `YH_AUTH_CONFIG_FILE` when needed for a local runnable build; do not commit credentials or include them in CI artifacts.

When changing build steps, resources, translations, or dependencies, make the Windows CI workflow exercise the same path. Run the available checks locally and state clearly which platform or network-dependent steps could not be verified.

## UI and translations

Keep pages focused on composing `Ui` and `Components` types, translated labels, bindings, and event wiring. Put general visual primitives in `src/qml/ui/` and app-specific reusable visuals in `src/qml/components/`; register them in `src/qml/CMakeLists.txt`. Reuse the shared theme, typography, and controls. See `docs/qml.md`.

Components receive display data through explicit properties and emit user intent through signals. They must not reach into page IDs or dispatch actions through hidden global models. Prefer typed required properties for page dependencies. Bind loading, error, and ready state to view-model properties; QML owns animations, dialogs, and local interaction state. Give navigation and authorization routing a single owner.

Visual extraction preserves the existing page/controller event wiring and behavior. Move application logic only when the task requests it; do not change polling, reconciliation, or session behavior as a side effect of a UI refactor.

Wrap user-facing QML and C++ strings for translation, preserve placeholders, and keep the English catalog complete. For bundled JSON display values, regenerate `translations/DataStrings.cpp` with `scripts/update-data-translation-markers.py`. See `docs/localization.md`.

## API and model boundaries

Follow `QML -> view models / display models -> services -> API interfaces`. Keep QML-facing state and commands in `src/models`, application operations in `src/services`, HTTP transport and Yandex API details in `src/api`, and authentication in `src/auth`. View models adapt service state for a page; list models expose rows, roles, and notifications. List models do not own HTTP requests or polling.

Services use the existing shared API interfaces, such as `IHomeApi`, with injected live or fake implementations. Do not add a per-feature gateway merely to wrap the same API. Keep GUI and CLI composition in `src/app`; reusable services must work without QML. Preserve QML-facing properties and signals while refactoring, and handle callback lifetime, request identity, and session reset at the layer that owns the operation. See `docs/device-control-architecture.md` and `docs/authorization-architecture.md` when touching those flows.

Add deterministic Qt tests for changed behavior; avoid live API calls in unit tests. For changed QML wiring, exercise the actual component or page and verify that user intent reaches the intended model or service. Use the checks in `docs/build.md` and state what could not be verified. Never commit credentials or tokens or include them in logs and test output.

## Shared workspace

Other agents may be working in this checkout. Inspect the working tree before editing, keep edits within the assigned scope, and preserve unrelated changes. Do not reset, stash, stage, or commit another agent's work as part of your task.
