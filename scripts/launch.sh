#!/usr/bin/env bash
set -euo pipefail

executable=$1
shift
# Keep Git Bash from rewriting app arguments that happen to look like Unix paths.
export MSYS2_ARG_CONV_EXCL='*'
for candidate in "$YH_JUST_BUILD_DIR/$YH_JUST_CONFIG/$executable" "$YH_JUST_BUILD_DIR/$executable"; do
    if [[ -f "$candidate" ]]; then
        # Prefix relative paths so Bash does not search PATH for the executable.
        exec "$(cd "$(dirname "$candidate")" && pwd)/$(basename "$candidate")" "$@"
    fi
done
echo 'Executable not found; enable the corresponding desktop, cli_app, or rest_app build option' >&2
exit 2
