# Connect 4 — Browser UI

A browser-playable version of the Connect4 engine from `../DSA-project`, compiled to
WebAssembly via [Emscripten](https://emscripten.org/). The original course sources
(`DSA-project/include`, `DSA-project/src`) are **not modified** — this folder only adds
a thin bridge (`c/wasm/web_bridge.c`) and a plain HTML/CSS/JS frontend on top.

## Layout

- `c/wasm/web_bridge.h` / `.c` — the only new C code. Exposes a small API
  (start/reset game, read the board, apply a move, ask the bot for a move) that the
  frontend calls into. Mirrors `DSA-project/src/interface.c`'s move-application and
  tree-reroot logic, but with no stdio.
- `c/wasm/web_bridge_smoke_test.c` — standalone native smoke test (`gcc`, no munit).
- `build/build.sh` / `build-debug.sh` — Emscripten build scripts.
- `public/` — static site: `index.html`, `style.css`, `main.js`, `board.js`,
  `bot-worker.js`. `public/vendor/connect4.js` + `.wasm` are build output (gitignored,
  built by CI — see `.github/workflows/deploy-web.yml`).

## Local development

Requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html)
installed and activated (pin version **3.1.69** to match CI):

```bash
# one-time setup
emsdk install 3.1.69
emsdk activate 3.1.69
source /path/to/emsdk/emsdk_env.sh   # or emsdk_env.bat on Windows

# build
bash web/build/build.sh          # release build
bash web/build/build-debug.sh    # debug build (ASSERTIONS, SAFE_HEAP)

# serve
npx http-server web/public -c-1
```

Then open the printed local URL and play a full game in both "vs Bot" and
"2-Player Local" modes.

## Native smoke test (no Emscripten needed)

```bash
gcc -IDSA-project/include -Iweb/c/wasm \
    DSA-project/src/connect4.c DSA-project/src/tree.c DSA-project/src/game_bot.c \
    web/c/wasm/web_bridge.c web/c/wasm/web_bridge_smoke_test.c \
    -lm -o smoke && ./smoke
```

## Deployment

`.github/workflows/deploy-web.yml` builds `public/` on every push to `main` and
publishes it via GitHub Pages (Settings → Pages → Source: GitHub Actions).

## Notes

- The bot's search (`best_move`, plain minimax with no alpha-beta pruning) runs in a
  dedicated Web Worker with its own WASM instance, so it never blocks the UI thread.
  The main thread's instance is kept in lockstep by replaying the same move sequence
  (see `main.js`/`bot-worker.js`) rather than by serializing the tree.
- Bot difficulty is user-selectable in the UI (Kolay/Orta/Zor/İmkansız = depth
  5/6/7/8). Depth 8 (`main.c`'s original default) has no alpha-beta pruning, so it
  can be noticeably slower — the Web Worker + "Bot is thinking..." indicator keep
  the UI responsive while it searches.
