---
name: yh-qml-composition
description: Implement, refactor, or review QML pages and reusable visuals in YandexHomeDesktop using explicit properties, intent signals, and view-model bindings. Use for layout, component extraction, delegates, and UI event wiring; C++ operation changes also use yh-application-architecture.
---

# YandexHomeDesktop QML composition

Keep pages readable compositions and components independently usable with supplied data. Apply this pattern to new code and requested refactors; it does not authorize a broader migration of legacy controls.

## Context to read

Read [docs/qml.md](../../../docs/qml.md) for module layout, navigation, and current wiring. Read [docs/localization.md](../../../docs/localization.md) when display text changes, and [docs/build.md](../../../docs/build.md) for the relevant verification commands. Inspect the affected page, its components, and their call sites before changing a contract.

## Choose the right owner

| Layer | Owns |
| --- | --- |
| `src/qml/pages/` | Composition, translated labels, bindings to page state, and wiring intent signals to commands or navigation |
| `src/qml/components/` | App-specific visuals such as device rows, headers, pickers, and list panes |
| `src/qml/ui/` | Shared surfaces, typography, buttons, dialogs, scrolling, and loading/error visuals |
| `src/qml/controls/` | Device capability/property presentation under the existing control contract |
| C++ view models and services | Application state and operations; use `yh-application-architecture` when changing them |

Reuse the established `Ui` and `Components` types, theme properties, typography, spacing, and assets. Move a meaningful visual responsibility into a component when it makes a page clearer or supports reuse; avoid extracting wrappers that add no useful contract. Pages should not accumulate custom rectangles, mouse areas, lists, or inline control delegates.

## Component contracts and state

- Supply models, display values, selection, and availability through explicit properties. Use typed required properties for dependencies where the existing registered types permit it.
- Emit intent signals with the identity and value the caller needs, such as `deviceSelected(deviceId)` or `favoriteToggleRequested(deviceId)`. The page wires these to its view model or navigation.
- Keep reusable components independent of page IDs, implicit delegate context, and global operation models. Theme access follows the existing shared theme convention.
- Pass required data explicitly through delegates and `Loader`s. Keep the owning model and item identity unambiguous; a nested control's `model` may refer to a different list. Preserve legacy row-index APIs until their migration is requested.
- Bind loading, error, ready, pending, and persistent selection state to C++ properties. Do not reconstruct operation state by counting signals or assigning visibility from API callbacks.
- Keep animations, open dialogs, hover/pressed state, and temporary visual selection in QML. `Connections` is appropriate for transient presentation events, such as opening an error dialog; it should not become an application workflow controller.
- Let `Main.qml` own authorization routing. Page handlers own feature navigation; reusable rows emit selection intent.

Visual extraction preserves the existing event handlers and observable behavior. Changing action dispatch, polling, reconciliation, or session reset is separate work under the application architecture skill.

## Packaging and translations

Register new module types in `src/qml/CMakeLists.txt`. URL-loaded pages and controls also need their entries in `resources/resources.qrc`; follow the current routing and delegate maps in the QML guide. Add bundled assets to the resource manifest.

Keep source display text in Russian with `qsTr()` or C++ `tr()`, preserve placeholders, and complete the English catalog. User/API-provided names are displayed as supplied. Regenerate `translations/DataStrings.cpp` only when bundled JSON display strings change.

## Verification

Use an existing configured build or the clean-build instructions in the build guide. Discover the registered tests with `ctest --test-dir <build-dir> -N`; run the affected page-load checks, `QmlLayouts`, and the relevant feature tests for changed wiring. Add a deterministic interaction test when behavior or ownership changes, using actual components and fake/local dependencies. A useful test clicks the intended control and verifies the intended owner receives the correct identity and value.

For layout changes, inspect the affected states and scrolling at relevant sizes. Use existing fixture data for manual UI checks when needed. Check QML warnings and translation checks where applicable, and report any unavailable platform or visual verification. Do not require new tests for a simple visual-only extraction when existing checks cover its preserved behavior.
