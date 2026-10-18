# Local REST API

The CLI controls a dedicated headless `YandexHomeRest` executable:

```sh
YandexHomeCli --enable-rest
YandexHomeCli --status-rest --json
YandexHomeCli --disable-rest
```

Enable starts a background process, waits for the listener, and exits. Disable
closes the listener, cancels pending response delivery, stops the process, and
exits after the worker finishes cleanup. Turning REST off during startup also
waits for the launching worker and stops it. A worker that fails to exit produces
a timeout error. Repeated enable/disable commands succeed. REST remains available after
the GUI closes. Settings → Advanced → REST API controls the same process; CLI
changes appear within two seconds. The live preference and port are saved.
An enabled server restarts when the GUI next restores a signed-in session.
This does not install an OS service or start REST at boot. GUI logout and CLI
reset disable the default REST profile.

Sign in through the GUI before enabling live REST. The background process
restores the saved login and may need approval of a system Keychain prompt.
It does not open a browser. Status and disable work without sign-in.
`--timeout <ms>` bounds a control command (default 30000 ms).

## Advanced JSON configuration

Pass `--rest-config /absolute/path/to/rest.local.json` to either console executable:

```json
{
  "host": "127.0.0.1",
  "port": 9000,
  "oauth_token": "YOUR_YANDEX_OAUTH_ACCESS_TOKEN"
}
```

```sh
YandexHomeCli --rest-config ./rest.local.json --enable-rest
YandexHomeCli --rest-config ./rest.local.json --status-rest --json
curl http://127.0.0.1:9000/v1/status
YandexHomeCli --rest-config ./rest.local.json --disable-rest

# Foreground mode:
YandexHomeRest --rest-config ./rest.local.json
```

All fields are optional. `host` defaults to `127.0.0.1` and accepts IPv4/IPv6
addresses or `localhost`; for example, `::1` binds IPv6 loopback. `port` is an
integer from 1 to 65535. An explicit `--rest-port` overrides the JSON value.
Without a port in the file, config profiles default to 8765 (8766 in fixture mode).

`oauth_token` is the Yandex OAuth access token, with the permissions needed for
the requested upstream operations. Supplying it skips desktop sign-in and all
secret-storage access. It is used through the existing API token providers and
is never copied to process arguments, settings, or status output. Omitting it
uses the existing saved-login flow. An empty or malformed supplied token causes
a configuration error; it does not fall back to secret storage. Upstream requests
report expired or rejected tokens through the normal API errors. Fixture mode
ignores a supplied token and continues to use local fixture data.

Use the same config path for enable, status, and disable. Each path has a separate
worker and control channel, with in-memory preferences; config profiles do not
change the GUI's REST preference or port. Changes take effect after disabling and
enabling that profile. Status and disable still work if the file is removed or
becomes invalid. The file is loaded at startup rather than watched for changes.

Start from [the credential-free example](../resources/rest/example.json). Keep a
file containing an actual token private; the repository ignores `rest.local.json`
at its root. A wildcard host (`0.0.0.0` or `::`) binds all corresponding interfaces.
Clients then use the machine's actual address. The listener uses HTTP and has no
client authentication, so a non-loopback host makes its controls reachable on
that network.

The default live profile listens on `127.0.0.1:8765`; Debug fixtures default to
`127.0.0.1:8766`. Select another port with `--enable-rest --rest-port 9000`.
Changing an active listener's port requires disabling first. Occupied ports
produce an error. Status includes `enabled`, `running`, `fixture`, `host`, `port`, and
`url`, and never includes a token. `enabled` is the saved preference and
`running` is the process state; a stopped process may still have an enabled
preference. `YandexHomeRest [--rest-port 9000]` runs in the foreground for a
terminal or process supervisor. It accepts the same fixture, JSON, timeout, and
control flags. The CLI directs users of `--serve-rest` to this executable. Disable stops this process too.

## Access and endpoints

The API requires no REST access token. The default listener binds to `127.0.0.1`.
It validates the `Host` header against the connection's destination address and
port (with `localhost` accepted for loopback), rejects browser Origin requests,
and disables CORS. A JSON profile can select a different bind address.
All POST requests require `Content-Type: application/json`. Responses use
`Cache-Control: no-store`. Yandex OAuth tokens remain internal to the worker.
The lifecycle channel uses a private per-user socket directory on Unix and
user-restricted local IPC on Windows.

Paths are relative to the status `url`, e.g. `http://127.0.0.1:8765/v1`.
Use exact entity IDs and URL-encode them as path segments.

| Method | Path | Result |
| --- | --- | --- |
| GET | `/openapi.json` | OpenAPI 3.1 specification for this listener |
| GET | `/status` | Running state and protocol version |
| GET | `/devices` | Device summaries, rooms, and households |
| GET | `/devices/{id}` | State, capabilities, parameters, and properties |
| POST | `/devices/{id}/actions` | Send one validated device action |
| GET | `/scenarios` | IDs, names, and active flags |
| POST | `/scenarios/{id}/run` | Run an active scenario |
| GET | `/account` | Account ID, display name, and email |

## OpenAPI specification

The committed [OpenAPI 3.1 document](../resources/rest/openapi.json) describes all
HTTP operations, entity-ID parameters, typed device actions, response schemas,
and error statuses, with examples for every supported action shape. Download it
from the running server:

```sh
curl http://127.0.0.1:8765/v1/openapi.json -o openapi.json
```

Use the configured host/port when different. The served document places the
active listener URL first in `servers`, including custom ports and IPv6 addresses.
The source document also includes editable host/port variables for offline use.
Import the JSON file into OpenAPI tooling to browse operations or generate clients.
Browser Origin requests remain rejected, so use the downloaded file with hosted tools.
The document is bundled in `AppRest`, works from any working directory, and is
also installed under `share/YandexHome/rest/openapi.json`.

The specification is returned as raw OpenAPI JSON, without an `ok` success
envelope. The normal JSON error envelope still applies to errors at this path.
The local REST API has no client authentication; the server's Yandex OAuth token
belongs in server configuration and is never returned in the document.

## Device actions and errors

Device actions require `Content-Type: application/json` and typed JSON:

```json
{"capability":"on_off","state":{"instance":"on","value":true}}
```

```json
{"capability":"range","state":{"instance":"brightness","value":50}}
```

```json
{"capability":"range","state":{"instance":"volume","value":-5,"relative":true}}
```

Names accept `on_off`, `range`, `mode`, `toggle`, `color_setting`, or full
`devices.capabilities.*` names. Booleans must be JSON booleans and numbers
finite. RGB uses a numeric integer, HSV an object with `h`, `s`, and `v`;
scene/mode values are strings. Supported instances, limits, modes, and colors
are checked through shared `IotCore` rules and device metadata. An acknowledgement
means the upstream API accepted the action. REST does not poll to confirm
physical state or automatically retry mutations. Scenario execution accepts
an empty body or `{}`.

Operation success contains `ok: true` (except the raw OpenAPI document); failure contains
`{"ok":false,"error":{"code":"...","message":"..."}}`. Codes are stable;
messages are translated. HTTP errors use 400 for invalid input, 401/403 for
authentication, 404 for missing targets/routes/capabilities, 405 for incorrect
methods, 409 for inactive scenarios or duplicate capabilities, 413 for bodies
larger than 64 KiB, 415 for non-JSON POST requests, 502 for upstream failures, and
504 for timeouts. Response delivery has a 30-second budget by default.
Canceling delivery cannot undo an action already sent upstream.

## Python and fixtures

The example uses Python 3.9+ and its standard library:

```sh
python3 examples/rest_client.py --cli /path/to/YandexHomeCli
python3 examples/rest_client.py --cli /path/to/YandexHomeCli --device-id lamp --on
python3 examples/rest_client.py --cli /path/to/YandexHomeCli --scenario-id evening
```

It retrieves status through the CLI, lists devices, and optionally reads/controls
a device or runs a scenario. HTTP requests need no key or authentication header.

For a credential-free Debug example:

```sh
YandexHomeCli --fake-api --enable-rest
python3 examples/rest_client.py --cli /path/to/YandexHomeCli --fake-api --device-id lamp
YandexHomeCli --fake-api --disable-rest
```

Pass the same `--fake-api-data /absolute/path/to/fixture.json` to the enable,
status, disable, and Python commands for a custom fixture. Each path
has a separate control channel and uses temporary preferences; fixtures never
access live OAuth credentials. Reads and scenario execution work. Device
actions return the existing fixture API's explicit "not simulated" error.
Release rejects `--fake-api` before any server starts.

## Architecture and verification

`RestServer` owns the HTTP listener, access checks, and in-flight replies. It
depends on `RestRouter`, whose route table declares methods, paths, and named
parameters. `RestEndpoints` adapts JSON requests and responses to the shared
Home, Device, Scenario, and Account services. `RestProtocol` names the protocol
version, body limit, and default request timeout. Qt HTTP Server parses HTTP;
each `RestReply` owns its callback context and timeout.

`DeviceService::ApplyCapability` validates commands against `IotCore` rules and
fresh device metadata. `ScenarioService::RunActiveScenario` checks that the
requested scenario exists and is active. These one-shot operations return
typed application rejections; the REST adapter maps them to HTTP errors. They
require no QML models or polling sessions and leave desktop session operations
unchanged.
`RestControlService` adapts REST settings, launch arguments, and status fields to
`WorkerControlClient`. The shared `WorkerControl` library owns local JSON IPC,
launch coordination, detached startup, and complete shutdown;
`WorkerControlServer` owns the worker endpoint and process lock. See
[worker-control.md](worker-control.md) for the lifecycle and race contract.
`RestViewModel` projects its lifecycle into settings. `src/app/rest/RestApp.cpp`
composes the headless worker; `src/app/rest/RestMain.cpp` owns its `QCoreApplication`.
The GUI links only lifecycle control, and the CLI links its own command adapter.
`src/app/rest/RestConfiguration.cpp` selects the sibling REST executable, with a build-tree
macOS bundle fallback. Shared libraries provide operations to all three processes. A process lock protects simultaneous starts and
allows recovery after a crash.

`Rest` tests check validation, inline/overlapping replies, errors, timeouts,
cancellation, and controller/model state with scripted APIs and temporary
preferences. Separate controlled test workers verify shutdown completion,
shutdown timeouts, and disable during startup before the control channel opens.
They also validate JSON profiles and Host checks for IPv4, IPv6, and wildcard listeners.
The same suite checks the bundled OpenAPI document against the committed source
and registered router methods/paths, resolves internal schema references, and
submits every documented action example through the real device service.
`RestProcess` checks the CLI controller and dedicated worker: background startup, status,
fixture HTTP reads/scenarios, shutdown, port conflicts, and Release fixture
rejection. Synthetic-token profiles test foreground/background startup without
secret storage or upstream requests in both Debug and Release, including port
overrides, host conflicts, token redaction, and shutdown with a missing config file.
Those workers also serve OpenAPI from a separate working directory and advertise
the configured listener address without exposing a supplied token.
`QmlLayouts` clicks the settings toggle through the real model and
controller with a scripted local daemon. These run in the Windows
Debug/Release/PCH matrix and existing macOS/portable CTest paths. Process tests also run the
committed Python example against the fixture server and verify simultaneous
starts, foreground shutdown, and reset. Headless builds with both executables enabled also run these tests and require Python 3.9+.
