# CI pins just 1.51.0. Windows recipes use Bash from Git for Windows.
set shell := ['bash', '-euo', 'pipefail', '-c']
set windows-shell := ['bash', '-euo', 'pipefail', '-c']
set script-interpreter := ['bash', '-euo', 'pipefail']
set positional-arguments
set quiet

config := env('YH_JUST_CONFIG', 'Debug')
cli_app := env('YH_JUST_CLI_APP', 'ON')
rest_app := env('YH_JUST_REST_APP', 'ON')
desktop := env('YH_JUST_DESKTOP', if os() == 'linux' { 'OFF' } else { 'ON' })
build_dir := env('YH_JUST_BUILD_DIR', 'build-just' / if desktop == 'OFF' { 'portable' / lowercase(config) } else { lowercase(config) })
qt_prefix := env('YH_JUST_QT_PREFIX', env('CMAKE_PREFIX_PATH', env('QT_ROOT_DIR', '')))
auth_config := env('YH_JUST_AUTH_CONFIG', env('YH_AUTH_CONFIG_FILE', 'resources/auth/example.json'))
pch := env('YH_JUST_PCH', 'ON')
jobs := env('YH_JUST_JOBS', '4')
generator := env('YH_JUST_GENERATOR', 'Ninja')

# Propagate overrides when a recipe invokes another recipe through just.
export YH_JUST_CONFIG := config
export YH_JUST_DESKTOP := desktop
export YH_JUST_CLI_APP := cli_app
export YH_JUST_REST_APP := rest_app
export YH_JUST_BUILD_DIR := build_dir
export YH_JUST_QT_PREFIX := qt_prefix
export YH_JUST_AUTH_CONFIG := auth_config
export YH_JUST_PCH := pch
export YH_JUST_JOBS := jobs
export YH_JUST_GENERATOR := generator

# List available commands (also the default for just).
help:
    just --list

# Configure the selected profile; append any extra CMake arguments.
[script]
configure *args:
    case "$YH_JUST_CONFIG" in
        Debug|Release|RelWithDebInfo|MinSizeRel) ;;
        *) echo 'config must be Debug, Release, RelWithDebInfo, or MinSizeRel' >&2; exit 2 ;;
    esac
    for value in "$YH_JUST_DESKTOP" "$YH_JUST_CLI_APP" "$YH_JUST_REST_APP" "$YH_JUST_PCH"; do
        case "$value" in
            ON|OFF) ;;
            *) echo 'desktop, cli_app, rest_app, and pch must be ON or OFF' >&2; exit 2 ;;
        esac
    done
    options=(-S . -B "$YH_JUST_BUILD_DIR" -G "$YH_JUST_GENERATOR"
        "-DCMAKE_BUILD_TYPE=$YH_JUST_CONFIG" -DBUILD_TESTING=ON
        "-DBUILD_DESKTOP_APP=$YH_JUST_DESKTOP"
        "-DBUILD_CLI_APP=$YH_JUST_CLI_APP" "-DBUILD_REST_APP=$YH_JUST_REST_APP"
        "-DYH_BUILD_REST_ADAPTER=$YH_JUST_REST_APP" "-DYH_ENABLE_PCH=$YH_JUST_PCH")
    if [[ -n "$YH_JUST_QT_PREFIX" ]]; then
        options+=("-DCMAKE_PREFIX_PATH=$YH_JUST_QT_PREFIX")
    fi
    if [[ "$YH_JUST_DESKTOP" == ON ]]; then
        case "$YH_JUST_AUTH_CONFIG" in
            /*|[A-Za-z]:[\\/]*) auth_path=$YH_JUST_AUTH_CONFIG ;;
            *) auth_path=$PWD/$YH_JUST_AUTH_CONFIG ;;
        esac
        options+=("-DYH_AUTH_CONFIG_FILE=$auth_path")
    fi
    exec cmake "${options[@]}" "$@"

# Build; configure once if needed. Extra arguments go to cmake --build.
build *args: _configure-if-needed
    cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --parallel "$YH_JUST_JOBS" "$@"

# Build and run CTest; append filters such as -R Authorization.
test *args: build
    ctest --test-dir "$YH_JUST_BUILD_DIR" -C "$YH_JUST_CONFIG" --output-on-failure --parallel "$YH_JUST_JOBS" "$@"

# Build and launch the desktop executable with extra app arguments.
run *args: build-gui
    bash scripts/launch.sh '{{ if os() == "macos" { "YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop" } else { "YandexHomeDesktop.exe" } }}' "$@"

# Launch the Debug GUI with local fixture data; accepts --fake-api-data.
[script]
demo *args:
    if [[ "$YH_JUST_CONFIG" != Debug || "$YH_JUST_DESKTOP" != ON ]]; then
        echo 'Fixture mode requires config=Debug and desktop=ON' >&2
        exit 2
    fi
    just build-gui
    exec bash scripts/launch.sh '{{ if os() == "macos" { "YandexHomeDesktop.app/Contents/MacOS/YandexHomeDesktop" } else { "YandexHomeDesktop.exe" } }}' --fake-api "$@"

# Build and run the console CLI, e.g. cli --fake-api devices list --json.
cli *args: build-cli
    bash scripts/launch.sh '{{ if os() == "windows" { "YandexHomeCli.exe" } else { "YandexHomeCli" } }}' "$@"

# Build and run the foreground REST worker, e.g. rest --fake-api --rest-port 9000.
rest *args: build-rest
    bash scripts/launch.sh '{{ if os() == "windows" { "YandexHomeRest.exe" } else { "YandexHomeRest" } }}' "$@"

# Build only the GUI and its background REST worker when enabled.
build-gui: _configure-if-needed
    cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --parallel "$YH_JUST_JOBS" --target appYandexHomeDesktop

# Build only the CLI and its background REST worker when enabled.
build-cli: _configure-if-needed
    cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --parallel "$YH_JUST_JOBS" --target YandexHomeCli

# Build only the headless REST worker.
build-rest: _configure-if-needed
    cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --parallel "$YH_JUST_JOBS" --target YandexHomeRest

# Build and install under the build directory; accepts --prefix and other flags.
install *args: build
    cmake --install "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --prefix "$(cd "$YH_JUST_BUILD_DIR" && pwd)/stage" "$@"

# Remove compiled outputs with CMake's clean target, retaining configuration.
clean:
    cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --target clean

# Format C++ startup code; pass files or folders to select a different scope.
format *paths:
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/clang-tools.py format "$@"

# Check C++ formatting without editing files.
format-check *paths:
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/clang-tools.py check "$@"

# Analyze startup code using this profile's compilation database (PCH must be OFF).
tidy *paths: build
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/clang-tools.py tidy --build-dir "$YH_JUST_BUILD_DIR" "$@"

# Check the committed data markers and English translation catalog.
translations-check:
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/update-data-translation-markers.py --check
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/check-translations.py

# Regenerate data markers and extract messages for a desktop profile.
[script]
translations-update:
    if [[ "$YH_JUST_DESKTOP" != ON ]]; then
        echo 'translations-update requires a desktop profile' >&2
        exit 2
    fi
    {{ if os() == "windows" { "python" } else { "python3" } }} scripts/update-data-translation-markers.py
    just _configure-if-needed
    exec cmake --build "$YH_JUST_BUILD_DIR" --config "$YH_JUST_CONFIG" --target update_translations

# Check formatting, translations, build, and run the tests for the selected profile.
check: format-check translations-check test

_configure-if-needed:
    if [[ ! -f "$YH_JUST_BUILD_DIR/CMakeCache.txt" ]]; then just configure; fi
