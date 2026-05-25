#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"

preset="${KLOGG_CMAKE_PRESET:-macos-arm64}"
variant="${1:-klogg}"
case "${variant}" in
  klogg | app)
    app_name="klogg.app"
    process_name="klogg"
    ;;
  portable | klogg_portable)
    app_name="klogg_portable.app"
    process_name="klogg_portable"
    ;;
  *)
    echo "Usage: $0 [klogg|portable]" >&2
    exit 2
    ;;
esac

target_app="/Applications/${app_name}"

close_running_app() {
  local name="$1"
  local attempts=0

  if ! pgrep -x "${name}" >/dev/null 2>&1; then
    return
  fi

  echo "Closing running ${name}..."
  pkill -TERM -x "${name}" >/dev/null 2>&1 || true

  while pgrep -x "${name}" >/dev/null 2>&1; do
    if (( attempts >= 40 )); then
      echo "Force closing ${name}..."
      pkill -KILL -x "${name}" >/dev/null 2>&1 || true
      break
    fi

    sleep 0.25
    attempts=$((attempts + 1))
  done

  attempts=0
  while pgrep -x "${name}" >/dev/null 2>&1; do
    if (( attempts >= 20 )); then
      echo "Unable to close running ${name}" >&2
      exit 1
    fi

    sleep 0.25
    attempts=$((attempts + 1))
  done
}

configure_output="$(cmake --preset "${preset}" -S "${repo_root}")"
printf '%s\n' "${configure_output}"

build_dir="$(printf '%s\n' "${configure_output}" \
  | sed -n 's/^-- Build files have been written to: //p' \
  | tail -n 1)"
if [[ -z "${build_dir}" ]]; then
  echo "Unable to determine CMake binary dir for preset: ${preset}" >&2
  exit 1
fi

cmake --build --preset "${preset}"

source_app="${build_dir}/output/${app_name}"
if [[ ! -d "${source_app}" ]]; then
  echo "Built app not found: ${source_app}" >&2
  exit 1
fi

close_running_app "${process_name}"
ditto "${source_app}" "${target_app}"
echo "Installed ${target_app}"
