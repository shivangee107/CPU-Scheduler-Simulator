#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="$ROOT_DIR/build/cpu_scheduler_simulator"
OUT_DIR="$ROOT_DIR/outputs"

if [[ ! -x "$BIN" ]]; then
  echo "Binary not found: $BIN"
  echo "Build first:"
  echo "  cmake -S . -B build"
  echo "  cmake --build build"
  exit 1
fi

mkdir -p "$OUT_DIR"

for file in "$ROOT_DIR"/data/valid/*.csv; do
  name="$(basename "$file" .csv)"
  echo "Running valid case: $name"
  "$BIN" --input "$file" --quantum 3 > "$OUT_DIR/${name}.out"
done

echo
echo "Done. Outputs written to: $OUT_DIR"
