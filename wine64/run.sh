#!/usr/bin/env bash
set -euo pipefail

PORT="${BROKER_PORT:-35866}"
DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
APP="${APP_PATH:-"$DIR/launcher.exe"}"
EXE="${TARGET_EXE:-Xgpro.exe}"
SHIM="${SHIM_DLL:-shim.dll}"
BROKER="${USB_BROKER_PATH:-"$DIR/usb-broker"}"
WINECMD="${WINECMD:-wine}"

have_nc() { command -v nc >/dev/null 2>&1; }

is_valid_port() {
  case $1 in
    ''|*[!0-9]*) return 1
  esac
  [ "$1" -ge 1 ] && [ "$1" -le 65535 ]
}

port_ready() {
  if ! is_valid_port "$PORT"; then
    printf 'Invalid PORT="%s" (expected 1..65535)\n' "$PORT" >&2
    exit 1
  fi

  if (exec 3<>"/dev/tcp/127.0.0.1/$PORT") 2>/dev/null; then
    exec 3>&- 3<&-
    return 0
  fi
  if have_nc && nc -z 127.0.0.1 "$PORT" >/dev/null 2>&1; then
    return 0
  fi
  return 1
}

wait_port() {
  local timeout_sec="${1:-8}" i
  for ((i=0; i<timeout_sec*10; i++)); do
    if port_ready; then return 0; fi
    sleep 0.1
  done
  return 1
}

start_broker() {
  if port_ready; then
    echo "usb-broker already running on 127.0.0.1:$PORT"
    return 0
  fi

  if [[ ! -x "$BROKER" ]]; then
    echo "ERROR: broker not executable: $BROKER" >&2
    exit 1
  fi

  echo "Starting usb-broker: $BROKER --port $PORT"
  chmod +x "$BROKER" || true
  "$BROKER" --port "$PORT" --quiet &
  local bpid=$!
  echo "usb-broker PID=$bpid"

  if ! wait_port 10; then
    echo "ERROR: usb-broker did not open port $PORT in time" >&2
    kill "$bpid" 2>/dev/null || true
    exit 1
  fi
}

run_app() {
  cd "$DIR"
  echo "Launching wine app: $EXE"
  "$WINECMD" "$APP" --exe "$EXE" --shim "$SHIM" -- "$@" 2>/dev/null
}

start_broker
run_app "$@"
