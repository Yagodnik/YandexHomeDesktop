# CI pins just 1.51.0. Python preserves subprocess arguments on every platform.
set shell := ['python3', '-c']
set windows-shell := ['python', '-c']
set positional-arguments
set quiet

config := env('YH_JUST_CONFIG', 'Debug')
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
export YH_JUST_BUILD_DIR := build_dir
export YH_JUST_QT_PREFIX := qt_prefix
export YH_JUST_AUTH_CONFIG := auth_config
export YH_JUST_PCH := pch
export YH_JUST_JOBS := jobs
export YH_JUST_GENERATOR := generator

# List available commands (also the default for just).
help:
    from scripts.dev import main; import sys; sys.exit(main('help', sys.argv[2:]))

# Configure the selected profile; append any extra CMake arguments.
configure *args:
    from scripts.dev import main; import sys; sys.exit(main('configure', sys.argv[2:]))

# Build; configure once if needed. Extra arguments go to cmake --build.
build *args:
    from scripts.dev import main; import sys; sys.exit(main('build', sys.argv[2:]))

# Build and run CTest; append filters such as -R Authorization.
test *args: build
    from scripts.dev import main; import sys; sys.exit(main('test', sys.argv[2:]))

# Build and launch the desktop executable with extra app arguments.
run *args: build
    from scripts.dev import main; import sys; sys.exit(main('run', sys.argv[2:]))

# Launch the Debug GUI with local fixture data; accepts --fake-api-data.
demo *args:
    from scripts.dev import main; import sys; sys.exit(main('demo', sys.argv[2:]))

# Build and run the console CLI, e.g. cli --fake-api devices list --json.
cli *args: build
    from scripts.dev import main; import sys; sys.exit(main('cli', sys.argv[2:]))

# Build and install under the build directory; accepts --prefix and other flags.
install *args: build
    from scripts.dev import main; import sys; sys.exit(main('install', sys.argv[2:]))

# Remove compiled outputs with CMake's clean target, retaining configuration.
clean:
    from scripts.dev import main; import sys; sys.exit(main('clean', sys.argv[2:]))

# Check the committed data markers and English translation catalog.
translations-check:
    from scripts.dev import main; import sys; sys.exit(main('translations-check', sys.argv[2:]))

# Regenerate data markers and extract messages for a desktop profile.
translations-update:
    from scripts.dev import main; import sys; sys.exit(main('translations-update', sys.argv[2:]))

# Check translations, build, and run the tests for the selected profile.
check: translations-check test
