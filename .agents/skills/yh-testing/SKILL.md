---
name: yh-testing
description: Discover, select, run, and maintain YandexHomeDesktop feature suites and individual test cases. Use when implementing or verifying changed behavior, refactoring tested code, investigating failures, or editing test registration/catalog metadata.
---

# YandexHomeDesktop testing

Use the repository catalog to connect changed behavior to runnable tests.
Read [docs/testing.md](../../../docs/testing.md) for selection/catalog details;
read [docs/build.md](../../../docs/build.md) when configuring a build is needed.

## Select coverage

Identify the changed observable behavior, then find the relevant tests:

```sh
just tests-list --suites --feature devices
just tests-list --feature devices
just tests-show DeviceControl
just tests-show DeviceControl::EmptyAttributesStillInitializeViewModel
```

Tags select complete suites conservatively. Include service/model tests and
the actual QML interaction or executable checks when the change crosses those
boundaries. A passing page-load check alone does not verify action wiring.
Add deterministic behavior tests for a meaningful gap; describe the guarantee
in the catalog in the same change.

## Run suites or concrete cases

Use an existing configured profile when suitable. Supply its actual
configuration; do not overwrite cached Qt/auth settings just to run a test.

```sh
just tests-run DeviceControl DeviceService
just tests-run --feature auth
just tests-run DeviceControl::EmptyAttributesStillInitializeViewModel
just tests-run QmlLayouts::PageComposition::test_deviceCommandThroughRealDelegate
just build_dir=build-rest-release config=Release tests-run RestProcess::JsonConfigurationAndExplicitTokenAvoidSecretStorage
```

C++ case IDs are `Suite::Function`; QML IDs are
`QmlLayouts::TestCase::test_function`. A data-driven function runs all its rows.
Use concrete cases for focused reproduction and diagnosis; verify a changed
feature with the affected complete suites. For shared-contract or test-tooling
changes, include all affected tests and desktop/portable profiles as appropriate.

`tests-run` builds first, validates selected runners against actual discovery,
then executes through CTest with the registered environment and timeout.
`--dry-run` resolves and validates a selection but does not execute tests.
Unknown/unavailable explicit IDs and empty selections are errors; do not replace
a failing selection with broader unrelated tests and call it verified.

Inspect profile availability with:

```sh
just tests-list --build-dir build-just/debug --config Debug --available
just desktop=OFF tests-run --feature devices
```

Desktop-only and Debug-only cases are recorded in the catalog. Feature runs
print unavailable suites; platform skips may also occur inside a Qt case.
Report those gaps with the run result. Discovery, metadata validation, and a
dry run are not evidence that assertions passed.

## Maintain discovery

When adding/renaming a case, update `tests/catalog.json` with its native name
and behavior description. Keep stable IDs for unchanged behavior. Register new
suites with `yh_add_test` so their feature/layer labels come from the catalog.
Custom Qt mains must forward discovery options and selectors after their own
arguments. Encode actual compilation conditions with `requires`.

```sh
just tests-check
```

This builds and checks suite names, labels, and discovered Qt cases. Normal
CTest also runs `TestCatalog` and `TestTooling`. For failed verification, retain
the failing behavior/expectation unless the requested change intentionally
changes it; explain the mismatch instead of weakening the test.

Report the selected suites/cases or features, build profile, pass/fail result,
and any unavailable or skipped checks. Link the changed tests when useful.
