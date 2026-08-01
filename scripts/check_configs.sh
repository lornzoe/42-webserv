#!/bin/sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT_DIR=$(CDPATH= cd -- "$SCRIPT_DIR/.." && pwd)
CHECKER="$ROOT_DIR/checker"
CONFIGS_DIR="$ROOT_DIR/configs"

if [ ! -x "$CHECKER" ]; then
	echo "error: $CHECKER not found or not executable (run 'make' first)" >&2
	exit 1
fi

FAIL_LOG=$(mktemp)
trap 'rm -f "$FAIL_LOG"' EXIT

count=0

find "$CONFIGS_DIR" -type f | sort | while IFS= read -r conf; do
	echo "==================================================================="
	echo "== $conf"
	echo "==================================================================="

	if "$CHECKER" "$conf"; then
		echo "--- OK: $conf"
	else
		echo "--- FAILED: $conf"
		echo "$conf" >> "$FAIL_LOG"
	fi
	echo
done

count=$(find "$CONFIGS_DIR" -type f | wc -l | tr -d ' ')
fail=$(wc -l < "$FAIL_LOG" | tr -d ' ')

echo "$count config(s) checked, $fail failed."

[ "$fail" -eq 0 ]
