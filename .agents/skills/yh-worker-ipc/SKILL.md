---
name: yh-worker-ipc
description: Implement or review YandexHomeDesktop local IPC and background-worker integration using the shared WorkerControl client/server. Use for new modules, fork integrations, worker commands, process ownership, readiness, and shutdown; HTTP endpoints use the REST layer.
---

# YandexHomeDesktop worker IPC

Reuse `WorkerControl` in `src/worker` for local JSON commands and detached worker
lifecycle. New features and forks should supply their own endpoint, executable,
commands, and application state rather than implement another local-server
wrapper, launcher, PID receipt, or ownership-lock scheme.

## Find the existing contract

Read [worker-control.md](../../../docs/worker-control.md) and the public headers
[WorkerControlClient.h](../../../src/worker/WorkerControlClient.h) and
[WorkerControlServer.h](../../../src/worker/WorkerControlServer.h).
Inspect the relevant consumer before changing shared behavior:

- REST: [RestControlService.cpp](../../../src/services/RestControlService.cpp),
  [RestConfiguration.cpp](../../../src/app/rest/RestConfiguration.cpp), and
  [RestApp.cpp](../../../src/app/rest/RestApp.cpp).
- Shortcuts: [ShortcutControlService.cpp](../../../src/services/ShortcutControlService.cpp),
  [ShortcutConfiguration.cpp](../../../src/app/shortcuts/ShortcutConfiguration.cpp),
  and [ShortcutMain.cpp](../../../src/app/shortcuts/ShortcutMain.cpp).
- External consumers: [sdk.md](../../../docs/sdk.md) and
  [examples/sdk](../../../examples/sdk).

Link the in-tree `WorkerControl` CMake target. An installed SDK consumer uses
`find_package(YandexHome CONFIG REQUIRED COMPONENTS WorkerControl)` and links
`YandexHome::WorkerControl`. Include `worker/WorkerControlClient.h` and
`worker/WorkerControlServer.h`; `WorkerControlPrivate.h` is an implementation detail.

Keep the shared library dependent only on Qt Core/Network. Feature services own
settings, translated errors, command validation, leases, and heartbeat policy;
composition roots in `src/app` choose launch configurations and worker programs.
Do not introduce GUI, REST, authentication, or hotkey dependencies into transport.

## Configure and call the client

`WorkerLaunchConfiguration` supplies `control_name`, `executable`, base
`arguments`, and `stop_command` (default `stop`). Share endpoint derivation
between controller and worker. Follow the existing configuration factories for
per-user identities, live/fixture/config isolation, short Unix socket paths, and
Windows local names. Give independent fork workers a distinct namespace when
they must coexist with upstream workers; retain the same identity only for
intentional compatibility.

Connect `finished` before calling `Request`, and establish application pending
state first: validation failures can finish inline. Keep the client alive and
use a QObject receiver context for completion handlers. `Request` is asynchronous
and has a default 30-second deadline; per-request launch arguments append to the
base arguments.

| Client command | Contract |
| --- | --- |
| `Start` | Probe with `status`, launch if absent, and complete only after `running: true`. |
| `Status` | Probe without launching; an absent worker is a successful non-running result. |
| `Stop` | Send `stop_command` and wait for actual process exit; absent workers are already stopped. |
| `Send` | Pass an application JSON object with a nonempty string `command`; this does not launch a worker. |

There is one active request per client, not a queue. `Stop` supersedes a pending
request; `Start` may replace `Status`; other overlapping requests are ignored.
`IsBusy()` excludes pending `Status` requests, so it is not an idle/queue-length
indicator. Keep any feature reconciliation or queuing in the owning service.
Replacement and destruction suppress old completions; a completion handler may
issue another request or destroy the client.

## Implement the worker side

Use `WorkerControlServer::Acquire(control_name)` before lengthy initialization,
install a handler with `SetHandler`, and call `Listen` when ready to serve control
requests. `Start(control_name)` combines acquisition and listening for workers
that can expose the endpoint immediately. Keep the server alive through cleanup.

The handler synchronously returns a `QJsonObject`; keep it short and run lengthy
application work asynchronously. Support `status` throughout startup and the
configured stop command. Successful lifecycle responses need boolean `ok` and
`running`; use `running: false` until application readiness, even when the socket
is listening. Application failures may return `ok: false` with `error.code` and
additional feature fields. The server attaches the actual process `pid`.

For stop, initiate graceful application shutdown, return `ok: true` and
`running: false`, and defer process exit long enough to send the reply, following
the existing worker entry points. A reply alone does not complete client Stop.
`Close()` closes sockets but retains ownership until server destruction; it
does not exit the process. A timeout or controller destruction does not stop a
detached worker. Reuse later `Stop` for recovery rather than deleting lock/receipt
files or terminating a PID obtained from a file or response.

Framing, bounds, socket permissions, transition locks, crash recovery, and launch
receipts belong to `src/worker`. Its protocol is one newline-terminated JSON
request/response per connection, bounded to 64 KiB in each direction, with a
five-second partial-request deadline. This is not a streaming or bulk-data
channel. Custom `Send` mutations are not replayed after ambiguous disconnects;
do not add feature retries that can execute them twice without an explicit
application-level idempotency contract. Raw local sockets belong in transport
implementation or scripted test peers, not another production feature wrapper.

## Verify the integration

Use the repository testing skill and [testing.md](../../../docs/testing.md) to
select suites. For transport/lifecycle changes run `WorkerControl` and affected
consumers (`Rest`/`RestProcess` or `Shortcuts`/`ShortcutProcess`). Changes to public
headers, linkage, or SDK exports also need `InstalledSdkConsumer`.

Use [WorkerControlTests.cpp](../../../tests/WorkerControlTests.cpp) as the
deterministic process/peer pattern: temporary endpoints, controlled child
processes, file gates, and bounded waits. For a new consumer, cover its command
payload and intended state transitions, startup before readiness, shutdown and
timeout behavior, and relevant concurrent/replaced requests. Keep unit/process
tests independent of live APIs, credentials, and native shortcut registration.

```sh
just tests-run WorkerControl Rest RestProcess
# For a shortcuts integration, select its consumers instead:
just tests-run WorkerControl Shortcuts ShortcutProcess
```

Preserve the shared ownership and cancellation guarantees when extending the
module; put reusable missing transport behavior there instead of copying it.
