# Device control: desktop and external changes

This records the existing behavior before extracting a device service. The
characterization tests in `tests/DeviceControlTests.cpp` exercise the production
`DeviceController`, `CapabilitiesModel`, `PropertiesModel`, and `DeviceDataModel`.
Surprising expectations are intentional compatibility observations, not proposed
fixes. Changing them should be a separate behavior decision.

## How external changes reach the desktop

The desktop receives device state through `GetDeviceInfo`. It has no mobile-app
events, writer identity, or conflict-resolution protocol. A mobile-app action is
therefore modeled as an independent change to the device state returned by the
next poll. These tests cover desktop reconciliation for selected server timelines;
they do not assert how Yandex or a particular physical device orders competing
writes.

Without a desktop action, every accepted device snapshot updates capability and
property rows. The controller does not compare `last_updated` or correlate read
responses by request ID. Replies arriving out of order can temporarily restore an
older value; a later poll can restore the current value.

## Desktop actions and suppression

`CapabilitiesModel::UseCapability` immediately replaces the row's displayed state
and emits `dataChanged`. The controller marks the row pending, records the action
start time, and sends exactly that capability to the selected device.
There is no immediate readback, rollback, or implemented `busy` role value.

For each incoming capability row, the controller skips the entire capability
object when any of these conditions holds:

1. The row is pending.
2. The response receipt time lies in `[action_start - 0.8, action_finish + 0.8)`.
3. The controller's most recent read start time lies in that same interval.

The bounds are measured in seconds. The lower bound is inclusive and the upper
bound is exclusive. Both successful and failed action callbacks clear pending and
record an action finish time. A skipped snapshot is not buffered or replayed when
the interval expires: a fresh accepted response must update the row.

Suppression is by row index, including when several capabilities have the same
type and different instances. It blocks changes to that row's parameters as well
as its value. Other capabilities, properties, and device metadata keep updating.
Raw `deviceDataReady` carries the full snapshot before capability suppression.
The `retrievable` flag does not alter this filtering behavior.

## Covered desktop/mobile timelines

| Timeline | Current desktop result |
| --- | --- |
| Mobile changes a capability while no desktop action is outstanding | The next accepted poll shows the mobile value. |
| Desktop changes a capability, then a conflicting mobile value is polled while the desktop request is pending | The desktop keeps its optimistic value. Other rows and properties update. |
| The server exposes the desktop write after the mobile write | Once suppression ends, an accepted poll retains the desktop value. |
| The server exposes the mobile write after the desktop write, before the desktop acknowledgement arrives | The optimistic desktop value remains through pending and the grace period, then a fresh poll shows the mobile value. |
| A mobile snapshot arrives during suppression and mobile changes again before the next accepted poll | Only the later accepted state is shown; the skipped snapshot is never replayed. |
| The desktop action fails while the server exposes a mobile value | An error is emitted. The optimistic value remains through the grace period; a fresh poll then replaces it. |
| A poll fails while a desktop action is pending | The action remains pending and its optimistic value stays protected. |
| Two different capability rows have outstanding desktop actions | Each row resumes independently when its own callback completes. |
| Two desktop actions target the same row | Either callback clears the shared pending flag, even if the other request remains outstanding. Mobile state can then overwrite the latest optimistic value. |
| Two polls return mobile snapshots in reverse order | The older reply can overwrite the newer value, including properties. |
| A poll started during an action returns late, with no newer poll started | Its capability is suppressed because the read start lies in the action interval. |
| The same late poll returns after another poll has started outside the interval | Its old capability can be accepted: the newer request overwrote the shared read start time. |
| An old-device snapshot arrives after selecting another device | Its ID mismatch prevents it from updating the selected device's rows. |

Coverage includes on/off, ranges, modes, toggles, RGB, HSV, switching from RGB to
temperature or scenes, non-retrievable capability entries, and the exact 800 ms
boundaries. Failure tests also record that `DeviceDataModel` marks itself offline
on an action error without emitting its online-state notification; a subsequent
successful read restores the online state.

## Deterministic test execution

The fake API captures a snapshot when a read is requested, records outgoing action
payloads, and delivers each callback only when the test chooses. Server writes and
action acknowledgements are separate events, so a late acknowledgement cannot
implicitly undo a simulated mobile action. The controller accepts an optional
time provider; its default is still `QDateTime::currentMSecsSinceEpoch() / 1000`.
Action start, finish, and read timestamps use this same provider.

Tests invoke the existing timer slot directly and stop the real timer after every
delivered read. They use no sleeps, live API, authentication, or hardware. They
assert the actual QML-facing state maps and model notifications, but do not drive
mouse clicks through the QML controls.

After a normal test build:

```sh
ctest --test-dir build -R '^DeviceControl$' --output-on-failure
```

The test is included in the regular CTest suite on Windows, macOS, and portable
builds. See `build.md` for configuration instructions.
