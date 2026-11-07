# Linux archive

The Linux desktop archive contains `YandexHomeDesktop`, the CLI, REST and MCP
workers, the project's shared libraries, and the C++ SDK. Extract it to a
writable directory and launch the app from a desktop session:

```sh
tar -xzf YandexHomeDesktop-<version>-linux-<architecture>.tar.gz
cd YandexHomeDesktop-<version>-linux-<architecture>
./bin/YandexHomeDesktop
./bin/YandexHomeCli --help --json
```

The desktop archive deploys the Qt libraries, plugins, and QML modules used by
the GUI. Use a compatible Linux system with the dependencies of your Qt
platform plugin, such as XCB libraries for X11 or XWayland. For live sign-in,
use a package built with working OAuth configuration and a desktop session
with a Secret Service keyring and `libsecret-1`. CI archives use the example
OAuth configuration. The app uses a normal window when Qt detects no system
tray at startup. Wayland may limit tray popup positioning. Global shortcuts
are unavailable on Linux.

### Console-only archive

A local archive built with `just desktop=OFF package-linux` contains the console
tools and SDK. Run them from `bin/`:

```sh
./bin/YandexHomeCli --help --json
./bin/YandexHomeRest --help --json
```

A matching shared Qt runtime is required: Qt Core, Network, HttpServer, and
WebSockets, plus the system dependencies of that Qt installation and
`libsecret-1`. The archive does not bundle Qt or the C/C++ system runtime.
If Qt is outside the system library search path, set `LD_LIBRARY_PATH` to
its `lib/` directory before launching an application. The project's own
shared libraries are found relative to the executables after extraction.

Official Linux release archives are built on Ubuntu 24.04 x86_64 with Qt
6.9.1. Use a compatible Linux system and Qt ABI. Locally generated archives
use the configured compiler and Qt kit.

The REST worker accepts an OAuth token through `--rest-config`; ordinary
device/scenario CLI commands use a previously saved login in the system
credential store. The CLI cannot start browser sign-in. See the repository's
`docs/cli.md` and `docs/rest.md` for configuration and commands.
