import Connect4Module from "./vendor/connect4.js";
import { buildBoard, renderBoard } from "./board.js";

// Depth 8 (main.c's original default) is too slow for a snappy web turn since
// best_move() runs plain minimax with no alpha-beta pruning. Depth 5 keeps the
// bot competent (eval_game_state's heuristic is informative) while staying fast.
const WIDTH = 7;
const HEIGHT = 6;
const TREE_DEPTH = 5;

const STATUS_LABELS = {
  0: "Player 1 (X) wins!",
  1: "Player 2 (O) wins!",
  2: "It's a draw.",
  3: null, // IN_PROGRESS: caller decides the message
};

const boardEl = document.getElementById("board");
const statusEl = document.getElementById("status-banner");
const modeSelect = document.getElementById("mode-select");
const newGameBtn = document.getElementById("new-game-btn");

let bridge = null; // main-thread wb_* bindings
let worker = null;
let columns = null;
let mode = modeSelect.value; // "bot" | "local"
let gameOver = false;
let thinking = false;
let pendingRequests = new Map();
let nextRequestId = 1;

async function initMainThreadModule() {
  const Module = await Connect4Module();
  return {
    startGame: Module.cwrap("wb_start_game", null, ["number", "number", "number"]),
    resetGame: Module.cwrap("wb_reset_game", null, []),
    getCell: Module.cwrap("wb_get_cell", "number", ["number", "number"]),
    getNextTurn: Module.cwrap("wb_get_next_turn", "number", []),
    getStatus: Module.cwrap("wb_get_status", "number", []),
    isMoveAvailable: Module.cwrap("wb_is_move_available", "number", ["number"]),
    applyMove: Module.cwrap("wb_apply_move", "number", ["number"]),
  };
}

function postToWorker(message) {
  return new Promise((resolve) => {
    const id = nextRequestId++;
    pendingRequests.set(id, resolve);
    worker.postMessage({ ...message, id });
  });
}

function setupWorkerListener() {
  worker.onmessage = (event) => {
    const { id } = event.data;
    const resolve = pendingRequests.get(id);
    if (resolve) {
      pendingRequests.delete(id);
      resolve(event.data);
    }
  };
}

function getCellChar(row, col) {
  return String.fromCharCode(bridge.getCell(row, col));
}

function isColumnFull(col) {
  return bridge.isMoveAvailable(col) === 0;
}

function refreshBoard() {
  renderBoard(columns, {
    width: WIDTH,
    height: HEIGHT,
    getCell: getCellChar,
    isColumnFull,
  });
}

function updateStatusBanner(status) {
  if (thinking) {
    statusEl.textContent = "Bot is thinking...";
    return;
  }
  const winMessage = STATUS_LABELS[status];
  if (winMessage) {
    statusEl.textContent = winMessage;
    return;
  }
  if (mode === "bot") {
    const humanTurn = bridge.getNextTurn() === 1; // player 2 ('O') is the human
    statusEl.textContent = humanTurn ? "Your turn (O)" : "Bot's turn (X)";
  } else {
    const nextIsPlayer2 = bridge.getNextTurn() === 1;
    statusEl.textContent = nextIsPlayer2 ? "Player 2's turn (O)" : "Player 1's turn (X)";
  }
}

function setBoardLocked(locked) {
  boardEl.classList.toggle("locked", locked);
}

async function triggerBotMoveIfNeeded() {
  if (mode !== "bot" || gameOver) return;
  if (bridge.getNextTurn() !== 0) return; // not the bot's (player 1's) turn

  thinking = true;
  setBoardLocked(true);
  updateStatusBanner(bridge.getStatus());

  const result = await postToWorker({ cmd: "botMove" });
  bridge.applyMove(result.column); // replay the bot's chosen column on the main instance
  thinking = false;

  refreshBoard();
  gameOver = result.status !== 3; // 3 === IN_PROGRESS
  setBoardLocked(gameOver);
  updateStatusBanner(result.status);
}

async function handleColumnClick(col) {
  if (gameOver || thinking) return;
  if (mode === "bot" && bridge.getNextTurn() !== 1) return; // wait for the bot
  if (isColumnFull(col)) return;

  const status = bridge.applyMove(col);
  if (status === -1) return; // illegal move, no state change

  refreshBoard();
  await postToWorker({ cmd: "applyMove", column: col });

  gameOver = status !== 3;
  setBoardLocked(gameOver);
  updateStatusBanner(status);

  if (!gameOver) {
    await triggerBotMoveIfNeeded();
  }
}

async function startNewGame() {
  mode = modeSelect.value;
  gameOver = false;
  thinking = false;

  bridge.startGame(WIDTH, HEIGHT, TREE_DEPTH);
  await postToWorker({ cmd: "start", width: WIDTH, height: HEIGHT, depth: TREE_DEPTH });

  setBoardLocked(false);
  refreshBoard();
  updateStatusBanner(bridge.getStatus());

  await triggerBotMoveIfNeeded();
}

async function main() {
  statusEl.textContent = "Loading engine...";

  bridge = await initMainThreadModule();
  worker = new Worker("./bot-worker.js", { type: "module" });
  setupWorkerListener();

  columns = buildBoard(boardEl, WIDTH, HEIGHT, handleColumnClick);

  newGameBtn.addEventListener("click", startNewGame);
  modeSelect.addEventListener("change", startNewGame);

  await startNewGame();
}

main();
