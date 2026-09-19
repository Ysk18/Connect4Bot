// Runs its own independent WASM instance so bot search (best_move, no alpha-beta
// pruning) never blocks the main/UI thread. Kept in lockstep with the main thread's
// instance purely by replaying the same sequence of moves (see main.js) -- the tree
// itself (a graph of raw pointers) is never serialized across the worker boundary.

import Connect4Module from "./vendor/connect4.js";

let wb = null;

async function ensureModule() {
  if (wb) return wb;
  const Module = await Connect4Module();
  wb = {
    startGame: Module.cwrap("wb_start_game", null, ["number", "number", "number"]),
    resetGame: Module.cwrap("wb_reset_game", null, []),
    applyMove: Module.cwrap("wb_apply_move", "number", ["number"]),
    doBotMove: Module.cwrap("wb_do_bot_move", "number", []),
    getStatus: Module.cwrap("wb_get_status", "number", []),
    getNodeCount: Module.cwrap("wb_get_node_count", "number", []),
  };
  return wb;
}

self.onmessage = async (event) => {
  const { cmd, id } = event.data;
  const bridge = await ensureModule();

  switch (cmd) {
    case "start": {
      const { width, height, depth } = event.data;
      bridge.startGame(width, height, depth);
      self.postMessage({ id, cmd, status: bridge.getStatus() });
      break;
    }
    case "reset": {
      bridge.resetGame();
      self.postMessage({ id, cmd, status: bridge.getStatus() });
      break;
    }
    case "applyMove": {
      const status = bridge.applyMove(event.data.column);
      self.postMessage({ id, cmd, status });
      break;
    }
    case "botMove": {
      const t0 = performance.now();
      const column = bridge.doBotMove();
      const elapsedMs = performance.now() - t0;
      self.postMessage({
        id,
        cmd,
        column,
        status: bridge.getStatus(),
        nodeCount: bridge.getNodeCount(),
        elapsedMs,
      });
      break;
    }
    default:
      self.postMessage({ id, cmd, error: `unknown command: ${cmd}` });
  }
};
