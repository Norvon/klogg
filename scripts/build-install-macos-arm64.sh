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

build_version="$(date '+%y.%m.%d')"
if [[ -f "${target_app}/Contents/Info.plist" ]]; then
  installed_version="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' \
    "${target_app}/Contents/Info.plist" 2>/dev/null || true)"
  if [[ "${installed_version}" =~ ^([0-9]+\.[0-9]+)\.([0-9]+)$ ]]; then
    # 同日重复打包也要更新 Finder 显示的三段版本号。
    next_version="${BASH_REMATCH[1]}.$(( 10#${BASH_REMATCH[2]} + 1 ))"
    build_version="$(printf '%s\n' "${build_version}" "${next_version}" | sort -V | tail -n 1)"
  fi
fi

configure_output="$(cmake --preset "${preset}" -S "${repo_root}" "-DBUILD_VERSION:STRING=${build_version}")"
printf '%s\n' "${configure_output}"

build_dir="$(printf '%s\n' "${configure_output}" \
  | sed -n 's/^-- Build files have been written to: //p' \
  | tail -n 1)"
if [[ -z "${build_dir}" ]]; then
  echo "Unable to determine CMake binary dir for preset: ${preset}" >&2
  exit 1
fi

# 先更新版本头文件，让增量构建重新编译版本库。
cmake --build --preset "${preset}" --target generate_version
cmake --build --preset "${preset}"

source_app="${build_dir}/output/${app_name}"
if [[ ! -d "${source_app}" ]]; then
  echo "Built app not found: ${source_app}" >&2
  exit 1
fi

built_version="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' \
  "${source_app}/Contents/Info.plist")"
if [[ "${built_version}" != "${build_version}" ]]; then
  echo "Built app version ${built_version} does not match expected version ${build_version}" >&2
  exit 1
fi

close_running_app "${process_name}"
ditto "${source_app}" "${target_app}"
echo "Installed ${target_app} (${built_version})"
