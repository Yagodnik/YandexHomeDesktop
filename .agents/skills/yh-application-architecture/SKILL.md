---
name: yh-application-architecture
description: Implement, refactor, or review YandexHomeDesktop C++ view models, list models, services, API/authentication boundaries, and asynchronous lifecycle behavior. Use when changing state ownership or application operations; QML composition alone uses yh-qml-composition.
---

# YandexHomeDesktop application architecture

Separate presentation from reusable application operations while preserving the behavior requested by the task. Use the existing shared API contracts with injected implementations.

## Context to read

Read [docs/qml.md](../../../docs/qml.md) for the current presentation wiring and [docs/build.md](../../../docs/build.md) for build/test commands. For device operations, read [device-control-architecture.md](../../../docs/device-control-architecture.md) and [device-control-behavior.md](../../../docs/device-control-behavior.md). For authentication or logout, read [authorization-architecture.md](../../../docs/authorization-architecture.md). Load only the flow-specific references needed by the task.

Inspect the affected service, view model, list model, composition root, and callers. Existing compatibility aliases and APIs are migration constraints, not an invitation to rewrite unrelated consumers.

## Ownership and dependency direction

```text
QML pages -> view models / display models -> application services -> API interfaces
CLI commands -----------------------------> application services -> API interfaces
                                                               -> live / fake implementations
```

| Layer | Responsibility |
| --- | --- |
| View models in `src/models` | QML-friendly commands, persistent page state, display-model ownership, and translated presentation errors |
| List/display models in `src/models` | Rows, roles, display values, and Qt change notifications |
| Services in `src/services` | Loading, validation, pending operations, request correlation, and cached application data; per-consumer sessions own selection/polling where the current design uses them |
| API interfaces/adapters in `src/api` | Yandex request/response details, serialization, and HTTP transport |
| Authentication in `src/auth` | Session/token contracts, injectable authentication core, and platform/OAuth adapters |
| Composition roots in `src/app` | Construct dependencies and choose GUI, CLI, live, or fixture implementations |

Use `HomeService`/`HomeViewModel` and `ScenarioService`/`ScenariosViewModel` as the existing patterns for page operations. Device operations use `DeviceService`, per-consumer `DeviceSession`, and `DeviceViewModel`; keep those distinct responsibilities.

List models must not issue HTTP requests, schedule polling, or decide request reconciliation. They may keep the existing optimistic display update and intent signal contracts while the view model routes requests to the session/service. Presentation choices such as titles, icons, translated errors, and QML delegate selection belong in the presentation layer.

Services depend directly on shared interfaces such as `IHomeApi` and `IAccountApi` when appropriate. A fake implementation supplies those same interfaces for tests/debugging. Do not introduce a per-feature gateway solely to forward the existing API methods. Add an abstraction only when it owns a distinct responsibility required by the task.

Keep reusable operations independent of QML and windows so GUI and CLI can share them. Reuse the Qt Core `IotCore` payload builders and validation rules rather than duplicating them in UI or CLI code. Authentication models expose UI commands/state without exposing tokens to QML; APIs obtain tokens through their existing provider contracts.

## Asynchronous behavior and incremental migration

- Give operation state one owner. View models project that state into observable properties; page recreation must see current state without relying on previously emitted initialization signals.
- Use stable entity IDs and request identity for new operations. Row indices remain presentation details; preserve an existing row-index contract until changing it is within scope.
- Preserve QObject callback-context lifetimes, including completion that may happen inline. Set up correlation and pending state before dispatch when the interface permits synchronous delivery.
- Keep session reset and stale-response handling at the owning service/session. Test logout and replacement requests when they affect the changed flow.
- Preserve existing QML properties, roles, signals, and compatibility aliases unless their migration is requested. Keep a single implementation of each operation behind any compatibility adapter.
- A structural refactor preserves documented device conflict behavior. Changes to suppression windows, rollback, ordering, late responses, or polling require an intentional behavior change with updated expectations; do not silently correct them while moving code.
- Keep account-specific persistence in the existing settings flow using stable account identity. Follow the current reset and fixture-isolation conventions; do not introduce UI-owned persistence.

## Verification

Use scripted API/authentication dependencies, temporary settings, and the injected clock where applicable. Test meaningful changed behavior: state transitions, duplicate requests, refresh/reordering during an operation, callback lifetime, inline completion, or logout with a pending response. No live API, OAuth, or credential-store access is needed in unit tests.

Run the relevant existing CTest suites from the build guide; for example, `Home`, `Scenarios`, `DeviceControl`, `DeviceService`, `Authorization`, or `OAuthAdapter`. Use desktop QML interaction checks when the change crosses the UI boundary and portable targets when verifying service independence. Follow `AGENTS.md` for reproducible builds and Windows CI coverage when build paths or dependencies change. State which platform/network-dependent checks were unavailable.
