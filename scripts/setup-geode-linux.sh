#!/usr/bin/env bash
set -euo pipefail

if ! command -v geode >/dev/null 2>&1; then
  echo "[1/3] Installing Geode CLI from source (cargo)..."
  cargo install --git https://github.com/geode-sdk/cli geode-cli
  export PATH="$HOME/.cargo/bin:$PATH"
else
  echo "[1/3] Geode CLI already installed"
fi

echo "[2/3] Installing/Updating Geode Linux SDK..."
geode sdk install

echo "[3/3] Building mod..."
cmake -S . -B build
cmake --build build -j"$(nproc)"

echo "Done. Built .geode package should be in build output directory."
