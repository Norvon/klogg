#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"

preset="${KLOGG_CMAKE_PRESET:-macos-arm64}"
variant="${1:-klogg}"
case "${variant}" in
  klogg | app)
    app_name="klogg.app"
    ;;
  portable | klogg_portable)
    app_name="klogg_portable.app"
    ;;
  *)
    echo "Usage: $0 [klogg|portable]" >&2
    exit 2
    ;;
esac

source_app="${repo_root}/build_root/macos-arm64/output/${app_name}"
target_app="/Applications/${app_name}"

cmake --preset "${preset}" -S "${repo_root}"
cmake --build --preset "${preset}"

if [[ ! -d "${source_app}" ]]; then
  echo "Built app not found: ${source_app}" >&2
  exit 1
fi

ditto "${source_app}" "${target_app}"
echo "Installed ${target_app}"
