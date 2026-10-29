# MCP server

The C++ MCP worker gives compatible AI agents access to the signed-in desktop
application's smart-home services. Keep the GUI open and signed in while using it.
The worker listens on IPv4 loopback at `http://127.0.0.1:8767/mcp` and uses
the shared local `WorkerControl` IPC channel to reach the GUI. It does not
depend on a running REST worker. The MCP listener is included when
`YH_BUILD_REST_ADAPTER` is enabled because it shares the HTTP listener library.

Enable or disable it in Settings → Advanced → MCP server, or use:

```sh
YandexHomeCli --enable-mcp
YandexHomeCli --status-mcp --json
YandexHomeCli --disable-mcp
```

Set the port in Settings → Advanced while the server is stopped, or use
`--enable-mcp --mcp-port <port>` to save a different port through the GUI's
control channel before starting. Subsequent starts use the saved port. Stop the
server before changing the port.

The JSON status contains `url` and `token` while the worker is running. Add the
URL to a client that supports MCP Streamable HTTP and configure its HTTP
`Authorization` header as `Bearer <token>`. Treat the token as a password: it
allows control of the home. A new token is generated each time the worker starts,
so update client configuration after a restart. The control channel is limited
to the current local user; other processes running as that user can request the
token. The listener rejects browser Origin requests and invalid Host headers.
It is not intended for network exposure.

For clients using an `mcpServers` JSON configuration with HTTP headers, the
entry has this shape (replace the token and use the client's documented syntax):

```json
{
  "mcpServers": {
    "yandex-home": {
      "url": "http://127.0.0.1:8767/mcp",
      "headers": {"Authorization": "Bearer <token>"}
    }
  }
}
```

The server supports MCP protocol version 2025-11-25 and exposes device and
scenario tools. The agent interprets the user's request; the tools provide
device names, aliases, rooms, capabilities and action validation. Ask for
clarification when multiple devices match. Successful action delivery means
the API accepted it, not that a physical device confirmed its new state.
On logout or loss of GUI authorization, pending operations are cancelled and
the worker stops. If the GUI exits unexpectedly, the worker detects its loss
within its heartbeat interval and stops.

The shared IPC channel limits each message to 64 KiB. A very large device or
home response can exceed that limit and fail; pagination is not yet available.
