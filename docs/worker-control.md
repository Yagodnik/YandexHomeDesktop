# Shared worker control

`WorkerControl` is a shared Qt Core/Network library in `src/worker`. It has no
dependency on settings, authentication, REST, QML, or a GUI application. REST and
shortcuts use the same client/server with their own endpoints, executables,
application commands, and readiness state. New modules and forks should follow
the [worker IPC skill](../.agents/skills/yh-worker-ipc/SKILL.md) to reuse it.

`WorkerControlClient` accepts Start, Stop, Status, and Send operations. A launch
configuration supplies the endpoint, executable, arguments, and stop command.
`WorkerControlServer` acquires process ownership, accepts local connections, and
dispatches complete JSON requests to an application handler. REST retains its
existing public commands, saved preferences, errors, and status fields through
`RestControlService`. Its worker retains ownership of HTTP and authorization.

## Ownership and concurrent commands

Two independent locks have different lifetimes:

- A controller transition lock serializes Start and Stop for one endpoint across
  threads of execution in separate clients/processes. A competing controller
  waits asynchronously within its request timeout. It probes the existing worker
  after acquiring the lock, rather than launching another copy.
- A worker ownership lock lasts until the server object is destroyed, including
  after its socket has closed. A second worker cannot acquire ownership or unlink
  the first worker's endpoint. Acquire ownership before lengthy initialization.

The lock's stale age is disabled: live long-running workers do not become stale
merely because time has passed. Qt's process-based stale-lock recovery handles
crashed owners. A private launch receipt records the child PID atomically, so
another controller can find a process before its control socket exists or after
the launching controller times out or exits. Foreground workers write the same
receipt. The receipt survives socket/server cleanup until process exit is
observed; a stale dead-process receipt is replaced on the next successful start.

Independent endpoint names have independent locks, receipts, and workers. REST
keeps its per-user live/config/fixture identities. Absolute Unix endpoints use a
private directory; Windows uses user-restricted local IPC. Short socket names
use a per-user cache directory for their lock files.

## Completion and cancellation

Start succeeds only when the application reports `running: true`. An available
socket with `running: false` is still starting. An observed child exit before
readiness produces `startup_error` without a respawn loop. A timeout is an error,
not cancellation of a detached process: its receipt allows a later Stop to find
it. Detached workers intentionally survive their controller's lifetime.

Stop supersedes a pending operation on the same client. Other Start/Stop callers
serialize at the transition lock. Stop waits for an acknowledged process to exit,
including when its endpoint has already closed; success means the process is no
longer running. Zombies awaiting their parent's reap count as exited. A worker
that does not exit produces a timeout, and a later Stop can retry. The client
never sends a termination signal to a PID read from a file or response.

Each operation owns its sockets and timers. Replaced operations cannot deliver a
completion into a newer request. Destruction cancels callback delivery. Completion
handlers may issue another request or destroy the client. Socket destruction is
deferred until an in-progress native read event returns.

## Local protocol

Requests and responses are newline-terminated JSON objects. Requests contain a
nonempty string `command`; application-specific fields pass through unchanged.
Responses contain a boolean `ok`; lifecycle success also contains a boolean
`running`. The server attaches its actual `pid`. Applications may return extra
status or error fields. Transport-generated errors contain `error.code`;
presentation adapters translate those codes.

One connection carries one request and response. Both directions are bounded to
64 KiB; partial requests have a five-second read deadline. Malformed requests do
not reach application handlers. Custom Send operations are not replayed after an
ambiguous disconnect, because an application mutation may already have happened.
An unanswered Send completes with `control_error` when the peer disconnects,
including when the platform emits no socket error alongside the disconnect.
Readiness probes and idempotent stop requests can be retried within one deadline.

## Verification

`WorkerControl` uses controlled child processes and file gates to hold startup,
readiness, socket closure, and process exit at specific points. It checks eight
simultaneous starter/stopping controllers, stop-before-listen, controller and
worker crashes, independent identities, shutdown timeouts, complete exit,
fragmented/malformed/oversized messages, cancellation, and reentrant destruction.
No live API, credentials, or hotkey registration is involved.

```sh
just tests-run WorkerControl Rest RestProcess
ctest --test-dir build-just/debug -R '^WorkerControl$' --repeat until-fail:10 --output-on-failure
```

`Rest`, `RestProcess`, and the existing QML settings interaction tests verify the
production REST adapter over the extracted implementation. `InstalledSdkConsumer`
checks installation/export of the shared libraries. Each CI platform runs the
worker suite once in Release; use the command above for repeated local checks.
