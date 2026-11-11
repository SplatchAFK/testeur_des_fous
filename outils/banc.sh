#!/usr/bin/env bash
set -eu

TDF_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BANC="${TDF_BANC:-$TDF_DIR/.banc}"

mkdir -p "$BANC/home"

if [ $# -eq 0 ]; then
	set -- bash "$TDF_DIR/run.sh" --version
fi

HOME="$BANC/home" TDF_NO_UPDATE=1 TDF_BANC_ACTIF=1 "$@"
