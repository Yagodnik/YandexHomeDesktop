# Global device and scenario shortcuts

Windows and macOS desktop builds include `YandexHomeShortcuts`, a separate Qt Gui
worker with no window, QML engine, or tray icon. **Global shortcuts** defaults to
enabled for new preferences; an explicitly disabled preference is preserved.
Saved bindings work after signing in while another application is
focused. Showing binding buttons and enabling execution are independent settings.
Assign scenario shortcuts using the key button beside its play button, with the
same recording, clearing, and conflict checks as device controls. Devices and
scenarios share one set of combinations per account. Renaming a scenario retains
its assignment because bindings use stable IDs.

By default, the service stops when the last GUI closes. **After closing the
application** keeps it running independently of tray mode. This does not install
an operating-system login item: after a reboot, open the GUI to start it again.
Disabling shortcuts or signing out stops the worker and releases its registrations.
With the GUI closed, `YandexHomeShortcuts --status --json` inspects it and
`YandexHomeShortcuts --stop --json` stops it. An open GUI restarts an unexpectedly
stopped worker while shortcuts remain enabled; use Settings to disable it.

The worker reads the saved token through the existing authentication service and
loads the account's stable ID independently. Tokens never enter IPC or arguments.
It accepts bindings only for that account. Changes, clears, and imports are sent
immediately; a two-second heartbeat also reconciles preferences changed by another
GUI. Invalid settings stop execution rather than leaving obsolete bindings active.
Machine-local execution/background preferences are excluded from account backups.

Only the worker registers native keys, using BSD-licensed
[QHotkey 1.5.0](https://github.com/Skycoder42/QHotkey/releases/tag/1.5.0), pinned to
commit `34330d6ff5d2ca111c376f6d7da66be9d1817430`. A checked CMake-generated
macOS compatibility patch makes Carbon registration exclusive and installs its
handlers once. The shared
[worker controller](worker-control.md) serializes concurrent starts/stops and waits
for process exit on explicit disable. Each GUI renews a ten-second lease. A
foreground worker exits when no clients remain, including after a GUI crash.
Background workers retain their registrations without a client.

Opening the binding editor unregisters shortcuts before accepting input. Recording
leases renew while it is open; closing or destroying it releases the pause.
Losing the GUI expires its recording lease even in background mode. The editor
rejects input when its acknowledgement becomes stale, before the worker's lease
expires. Multiple GUIs have independent recording leases. Native registration
conflicts appear in Settings; other bindings remain usable. System-reserved keys
may be unavailable. A changed keyboard layout or a released reservation may require
disabling and enabling shortcuts again.

Actions use `DeviceService` and `IotCore` without selecting a page or polling.
Each activation reads fresh device metadata. Boolean shortcuts toggle an actual
retrievable boolean state; offline/unavailable/ambiguous capabilities are rejected.
Ranges send relative changes using advertised precision (one when unspecified).
Modes, scenes, temperatures, and RGB/HSV colors are checked against the current
capability. Only one command per device can be pending. Reload, pause, stop, and
timeout invalidate operation contexts and stale callbacks. A write already
accepted by the remote API cannot be recalled.

Scenario actions use `ScenarioService::RunActiveScenario`. Every activation loads
the current scenario list and checks the saved ID and active flag before running.
Missing/inactive scenarios and API failures are reported without dispatching an
invalid run. One execution per scenario may be pending, independently of device
commands; cancellation and deadlines cover both lookup and execution.

IPC uses the same-user local worker channel and bounded JSON framing.
Configurations accept at most 256 bindings and 60 KiB. Status contains bounded
conflict details, registration/action counts, and error codes, never credentials.
The channel is intended for the current desktop user, not a remote API.

Debug `--fake-api [--fake-api-data file]` workers use the fixture API and fake
registrar, so tests/demo sessions never claim native shortcuts or read tokens.
Release rejects fixture options. Fixture `activate` IPC is compiled out of Release.
`Shortcuts` covers the portable core; `ShortcutProcess` exercises detached workers
and GUI controllers. Native keyboard delivery still needs platform testing with
another application focused.
