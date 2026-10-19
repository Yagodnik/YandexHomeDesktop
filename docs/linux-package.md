# Linux archive

This archive contains the headless `YandexHomeCli` and `YandexHomeRest`
applications, the project's shared libraries, and the C++ SDK. Extract it to
a writable directory and run the applications from `bin/`:

```sh
tar -xzf YandexHomeDesktop-<version>-linux-<architecture>.tar.gz
cd YandexHomeDesktop-<version>-linux-<architecture>
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
