#!/usr/bin/env bash
# Release WASM build. Requires the Emscripten SDK to be activated (emsdk_env.sh sourced).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DSA="$ROOT/DSA-project"
WEB="$ROOT/web"

mkdir -p "$WEB/public/vendor"

emcc \
  "$DSA/src/connect4.c" \
  "$DSA/src/tree.c" \
  "$DSA/src/game_bot.c" \
  "$WEB/c/wasm/web_bridge.c" \
  -I"$DSA/include" \
  -I"$WEB/c/wasm" \
  -O3 \
  -s WASM=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME=Connect4Module \
  -s EXPORTED_FUNCTIONS='["_wb_start_game","_wb_reset_game","_wb_get_width","_wb_get_height","_wb_get_cell","_wb_get_next_turn","_wb_get_status","_wb_is_move_available","_wb_do_human_move","_wb_apply_move","_wb_do_bot_move","_wb_get_node_count"]' \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s ENVIRONMENT=web,worker \
  -s EXPORT_ES6=1 \
  -o "$WEB/public/vendor/connect4.js"

echo "Build complete: web/public/vendor/connect4.js + connect4.wasm"
