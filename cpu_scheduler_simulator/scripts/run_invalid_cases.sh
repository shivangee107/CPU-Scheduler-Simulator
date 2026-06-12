#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT_DIR/build/cpu_scheduler_simulator"
OUT_DIR="$ROOT_DIR/outputs/invalid"

if [[ ! -x "$BIN" ]]; then
  echo "Binary not found: $BIN"
  echo "Build first:"
  echo "  cmake -S . -B build"
  echo "  cmake --build build"
  exit 1
fi

mkdir -p "$OUT_DIR"

for file in "$ROOT_DIR"/data/invalid/*.csv; do
  name="$(basename "$file" .csv)"
  echo "Running invalid case: $name"
  if "$BIN" --input "$file" > "$OUT_DIR/${name}.out" 2>&1; then
    echo "ERROR: invalid case unexpectedly succeeded: $file"
    exit 1
  fi
done

echo
echo "Done. All invalid cases failed as expected. Error outputs written to: $OUT_DIR"
