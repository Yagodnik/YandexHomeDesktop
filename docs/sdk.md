# Shared libraries

GUI, CLI, and REST have independent entry points and share the existing modules.
Install the SDK with `cmake --install build --prefix /path/to/sdk --component SDK`.
The package includes public headers, exported targets, shared libraries, and
the pinned Boost.Hana headers used by API model contracts.

```cmake
find_package(YandexHome CONFIG REQUIRED COMPONENTS AppServices YandexApiAdapter)
target_link_libraries(my_tool PRIVATE
    YandexHome::AppServices YandexHome::YandexApiAdapter)
```

Configure a consumer with `-DCMAKE_PREFIX_PATH="/path/to/sdk;/path/to/Qt/kit"`.
No source checkout, ignored libraries, credentials, or network fetch is required.
Use the same SDK release, Qt kit, compiler ABI, architecture, and build configuration;
the public C++ API is still evolving and has no long-term binary compatibility promise.
The installed package requires Qt 6.9+ and propagates C++23 to consumers.
Windows consumers must put the SDK's `bin/` and matching Qt runtime on `PATH`.
macOS/Linux consumers must ship the shared libraries with appropriate loader paths
or use the installed SDK/Qt runtime.

| Target | Responsibility |
| --- | --- |
| `YandexHome::ApiContracts` | Header-only API interfaces, results, and model types |
| `YandexHome::YandexApiAdapter` | HTTP transport, live APIs, and shared model codecs |
| `YandexHome::IotCore` | Capability payloads, metadata, and validation |
| `YandexHome::AppServices` | Home, device, scenario, account, and consumer sessions |
| `YandexHome::AuthContracts` | Token/session and authorization flow contracts |
| `YandexHome::AuthCore` | Session lifecycle over injected token store/flow |
| `YandexHome::YandexAuth` | Platform credential store and composition factory |
| `YandexHome::YandexOAuth` | Optional browser OAuth adapter, built with the GUI |
| `YandexHome::AppRuntime` | Settings and logging without GUI dependencies |
| `YandexHome::AppRestControl` | Asynchronous REST process control over local IPC |
| `YandexHome::AppCli` | Command registry, argument conversion, and command runner |
| `YandexHome::AppRest` | Optional HTTP listener, routing, and service mapping |

Shared libraries reuse code. Each process owns its own services, callbacks,
sessions, and caches. API interfaces accept caller-owned QObject contexts;
implementations suppress asynchronous delivery after a context is destroyed.
Pass an injected `IHomeApi` or `IAccountApi` to services, or construct the live
adapters with a token provider and HTTP transport. Inject stores/flows into
`AuthorizationService` for custom authentication.

`examples/sdk` is a complete external consumer. It uses a scripted API to load
a home, checks shared model codecs and capability builders, and constructs
the live adapter, auth core, CLI registry, and optional REST server without
making requests. CTest's `InstalledSdkConsumer` builds and runs it against only
the staged SDK. GUI models/QML/platform helpers and executable startup code
remain private; a future MCP adapter can consume the same public targets.
