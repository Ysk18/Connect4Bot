import Connect4Module from "./vendor/connect4.js";
import { buildBoard, renderBoard, updateColumnFullStates, dropPiece } from "./board.js?v=5";
import { playDropSound, playChime, unlockAudio } from "./audio.js?v=5";

const WIDTH = 7;
const HEIGHT = 6;
const DEFAULT_LOCAL_DEPTH = 5; // unused for move quality in 2-player mode, just needs a valid int

const PIECE_COLORS = {
  red: { name: "Kırmızı", light: "#ff8a8a", base: "#ef3b3b", dark: "#a11f1f" },
  yellow: { name: "Sarı", light: "#ffe9a3", base: "#ffcc33", dark: "#c98f00" },
  blue: { name: "Mavi", light: "#9ecbff", base: "#3aa0ff", dark: "#1c78d6" },
  green: { name: "Yeşil", light: "#a8e6b8", base: "#2ecc71", dark: "#1e8449" },
  purple: { name: "Mor", light: "#d7bde2", base: "#9b59b6", dark: "#6c3483" },
  orange: { name: "Turuncu", light: "#ffd8a8", base: "#ff8c1a", dark: "#cc6f00" },
  white: { name: "Beyaz", light: "#ffffff", base: "#eef0f2", dark: "#b9bec4" },
  black: { name: "Siyah", light: "#5a5a5a", base: "#2b2b2b", dark: "#0f0f0f" },
};

const BOARD_COLORS = {
  blue: { name: "Mavi", base: "#1f5fbf", dark: "#123a78" },
  green: { name: "Yeşil", base: "#1f7a3d", dark: "#0f4a24" },
  wood: { name: "Kahverengi", base: "#7a4a2b", dark: "#5c371f" },
  purple: { name: "Mor", base: "#5b2a86", dark: "#371756" },
  dark: { name: "Antrasit", base: "#33363d", dark: "#1c1e22" },
};

const boardEl = document.getElementById("board");
const statusEl = document.getElementById("status-banner");
const newGameBtn = document.getElementById("new-game-btn");

const chooseBotBtn = document.getElementById("choose-bot-btn");
const chooseLocalBtn = document.getElementById("choose-local-btn");
const botBackBtn = document.getElementById("bot-back-btn");
const botStartBtn = document.getElementById("bot-start-btn");
const colorErrorEl = document.getElementById("color-error");

let bridge = null; // main-thread wb_* bindings
let worker = null;
let columns = null;
let mode = null; // "bot" | "local"
let humanPlayerId = 0; // 0 = player 1 ('X'), 1 = player 2 ('O') -- which side the human plays, in "bot" mode
let selectedDepth = 6; // default: Orta
let selectedStarter = "human"; // "human" | "bot"
let boardColorKey = "blue";
let player1ColorKey = "red";
let player2ColorKey = "yellow";
let gameOver = false;
let thinking = false;
let thinkingInterval = null;
let pendingRequests = new Map();
let nextRequestId = 1;
let columnHeights = new Array(WIDTH).fill(0); // number of pieces currently stacked in each column

function showScreen(id) {
  document.querySelectorAll(".screen").forEach((el) => {
    el.classList.toggle("hidden", el.id !== id);
  });
}

function setupOptionGroup(containerId, dataAttr, initialValue, onChange) {
  const container = document.getElementById(containerId);
  const buttons = Array.from(container.querySelectorAll(".option-btn"));

  function select(value) {
    buttons.forEach((btn) => {
      btn.classList.toggle("selected", btn.dataset[dataAttr] === String(value));
    });
    onChange(value);
  }

  buttons.forEach((btn) => {
    btn.addEventListener("click", () => select(btn.dataset[dataAttr]));
  });

  select(initialValue);
}

// Builds a row of round color swatches from a palette map ({key: {name, base, ...}}),
// wires click-to-select, and calls onChange(key) whenever the selection changes
// (including once immediately with the initial value).
function setupSwatchGroup(containerId, palette, initialKey, onChange) {
  const container = document.getElementById(containerId);
  container.innerHTML = "";

  const buttons = new Map();
  for (const [key, color] of Object.entries(palette)) {
    const btn = document.createElement("button");
    btn.type = "button";
    btn.className = "swatch-btn";
    btn.style.background = color.base;
    btn.title = color.name;
    btn.dataset.colorKey = key;
    btn.addEventListener("click", () => select(key));
    container.appendChild(btn);
    buttons.set(key, btn);
  }

  function select(key) {
    for (const [k, btn] of buttons) {
      btn.classList.toggle("selected", k === key);
    }
    onChange(key);
  }

  select(initialKey);
}

function applyColorTheme() {
  const root = document.documentElement.style;
  const p1 = PIECE_COLORS[player1ColorKey];
  const p2 = PIECE_COLORS[player2ColorKey];
  const board = BOARD_COLORS[boardColorKey];

  root.setProperty("--p1-light", p1.light);
  root.setProperty("--p1-base", p1.base);
  root.setProperty("--p1-dark", p1.dark);
  root.setProperty("--p2-light", p2.light);
  root.setProperty("--p2-base", p2.base);
  root.setProperty("--p2-dark", p2.dark);
  root.setProperty("--board-base", board.base);
  root.setProperty("--board-dark", board.dark);
}

function colorsAreValid() {
  return player1ColorKey !== player2ColorKey;
}

function showColorError(show) {
  colorErrorEl.classList.toggle("hidden", !show);
}

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

function isHumanTurn() {
  return bridge.getNextTurn() === humanPlayerId;
}

function refreshBoard() {
  renderBoard(columns, {
    width: WIDTH,
    height: HEIGHT,
    getCell: getCellChar,
    isColumnFull,
  });
}

// Animates the piece that was just placed by `mover` (0 = player 1/'X',
// 1 = player 2/'O') falling into `col`, plays a timed landing thud, and
// refreshes the "column full" highlighting.
function dropAnimatedPiece(col, mover) {
  const row = HEIGHT - 1 - columnHeights[col];
  columnHeights[col]++;

  const symbol = mover === 0 ? "X" : "O";
  const { duration, whenLanded } = dropPiece(columns, row, col, symbol);
  setTimeout(playDropSound, duration * 1000 * 0.72);

  updateColumnFullStates(columns, WIDTH, isColumnFull);
  return { duration, whenLanded };
}

function startThinkingAnimation() {
  let dots = 0;
  statusEl.textContent = "Bot düşünüyor";
  thinkingInterval = setInterval(() => {
    dots = (dots + 1) % 4;
    statusEl.textContent = "Bot düşünüyor" + ".".repeat(dots);
  }, 400);
}

function stopThinkingAnimation() {
  if (thinkingInterval) {
    clearInterval(thinkingInterval);
    thinkingInterval = null;
  }
}

function getWinLabel(status) {
  if (status === 0) return `1. Oyuncu (${PIECE_COLORS[player1ColorKey].name}) kazandı!`;
  if (status === 1) return `2. Oyuncu (${PIECE_COLORS[player2ColorKey].name}) kazandı!`;
  if (status === 2) return "Berabere!";
  return null; // IN_PROGRESS
}

function updateStatusBanner(status) {
  if (thinking) return; // the thinking animation owns the banner text while active

  const winMessage = getWinLabel(status);
  if (winMessage) {
    statusEl.textContent = winMessage;
    playChime();
    return;
  }

  if (mode === "bot") {
    const youAreColor = humanPlayerId === 0 ? PIECE_COLORS[player1ColorKey].name : PIECE_COLORS[player2ColorKey].name;
    statusEl.textContent = isHumanTurn() ? `Sıra sende (${youAreColor})` : "Bot'un sırası";
  } else {
    const nextIsPlayer2 = bridge.getNextTurn() === 1;
    statusEl.textContent = nextIsPlayer2
      ? `2. Oyuncunun sırası (${PIECE_COLORS[player2ColorKey].name})`
      : `1. Oyuncunun sırası (${PIECE_COLORS[player1ColorKey].name})`;
  }
}

function setBoardLocked(locked) {
  boardEl.classList.toggle("locked", locked);
}

// Locks the board whenever clicking wouldn't make sense: game over, or (in
// bot mode) it's currently the bot's turn.
function refreshLockState() {
  setBoardLocked(gameOver || (mode === "bot" && !isHumanTurn()));
}

// Used only for the very first move of a game where the bot starts
// ("Bot başlasın") -- there's no preceding human drop animation to wait for,
// so the bot's move is shown as soon as it's ready.
async function triggerBotMoveIfNeeded() {
  if (mode !== "bot" || gameOver) return;
  if (isHumanTurn()) return;

  thinking = true;
  refreshLockState();
  startThinkingAnimation();

  const mover = bridge.getNextTurn(); // capture the bot's own symbol before applying
  const result = await postToWorker({ cmd: "botMove" });
  bridge.applyMove(result.column); // replay the bot's chosen column on the main instance

  thinking = false;
  stopThinkingAnimation();

  dropAnimatedPiece(result.column, mover);
  gameOver = result.status !== 3; // 3 === IN_PROGRESS
  refreshLockState();
  updateStatusBanner(result.status);
}

// Starts the bot's computation on the worker immediately (so slow, high-depth
// searches overlap with the human's drop animation instead of adding to the
// wait), but only reveals the bot's move -- visually dropping its piece --
// once the human's own piece has finished falling. This keeps the two drops
// from ever happening on screen at the same time.
async function runBotTurnAfterHumanDrop(syncPromise, humanWhenLanded) {
  const botMoverId = bridge.getNextTurn(); // bot's symbol id, captured now (won't change until we apply its move)

  let botDone = false;
  let botResult = null;
  const botPromise = syncPromise
    .then(() => postToWorker({ cmd: "botMove" }))
    .then((result) => {
      botDone = true;
      botResult = result;
      return result;
    });

  await humanWhenLanded; // the human piece's fall animation has actually finished playing

  if (!botDone) {
    // The bot is still thinking after the human's piece has landed --
    // only now do we show the "thinking" indicator.
    thinking = true;
    refreshLockState();
    startThinkingAnimation();
    botResult = await botPromise;
    thinking = false;
    stopThinkingAnimation();
  }

  bridge.applyMove(botResult.column); // replay the bot's chosen column on the main instance
  dropAnimatedPiece(botResult.column, botMoverId);

  gameOver = botResult.status !== 3; // 3 === IN_PROGRESS
  refreshLockState();
  updateStatusBanner(botResult.status);
}

async function handleColumnClick(col) {
  if (gameOver || thinking) return;
  if (mode === "bot" && !isHumanTurn()) return;
  if (isColumnFull(col)) return;

  const mover = bridge.getNextTurn(); // capture before the move flips next_turn
  const status = bridge.applyMove(col);
  if (status === -1) return; // illegal move, no state change

  const { whenLanded } = dropAnimatedPiece(col, mover);
  gameOver = status !== 3;
  refreshLockState();
  updateStatusBanner(status);

  const syncPromise = postToWorker({ cmd: "applyMove", column: col });

  if (!gameOver && mode === "bot") {
    await runBotTurnAfterHumanDrop(syncPromise, whenLanded);
  } else {
    await syncPromise;
  }
}

async function startNewGame() {
  gameOver = false;
  thinking = false;
  stopThinkingAnimation();
  columnHeights = new Array(WIDTH).fill(0);

  const depth = mode === "bot" ? selectedDepth : DEFAULT_LOCAL_DEPTH;

  bridge.startGame(WIDTH, HEIGHT, depth);
  await postToWorker({ cmd: "start", width: WIDTH, height: HEIGHT, depth });

  setBoardLocked(false);
  refreshBoard();
  updateStatusBanner(bridge.getStatus());

  await triggerBotMoveIfNeeded();
}

function tryProceedFromLanding(action) {
  if (!colorsAreValid()) {
    showColorError(true);
    return;
  }
  showColorError(false);
  action();
}

async function beginBotGame() {
  unlockAudio();
  mode = "bot";
  humanPlayerId = selectedStarter === "human" ? 0 : 1;
  showScreen("screen-game");
  await startNewGame();
}

async function beginLocalGame() {
  unlockAudio();
  mode = "local";
  humanPlayerId = 0; // unused in local mode
  showScreen("screen-game");
  await startNewGame();
}

async function main() {
  statusEl.textContent = "Yükleniyor...";

  bridge = await initMainThreadModule();
  worker = new Worker("./bot-worker.js?v=5", { type: "module" });
  setupWorkerListener();

  columns = buildBoard(boardEl, WIDTH, HEIGHT, handleColumnClick);

  setupOptionGroup("difficulty-group", "depth", selectedDepth, (v) => {
    selectedDepth = Number(v);
  });
  setupOptionGroup("starter-group", "starter", selectedStarter, (v) => {
    selectedStarter = v;
  });

  setupSwatchGroup("board-color-group", BOARD_COLORS, boardColorKey, (key) => {
    boardColorKey = key;
    applyColorTheme();
  });
  setupSwatchGroup("player1-color-group", PIECE_COLORS, player1ColorKey, (key) => {
    player1ColorKey = key;
    applyColorTheme();
    showColorError(false);
  });
  setupSwatchGroup("player2-color-group", PIECE_COLORS, player2ColorKey, (key) => {
    player2ColorKey = key;
    applyColorTheme();
    showColorError(false);
  });

  chooseBotBtn.addEventListener("click", () => tryProceedFromLanding(() => showScreen("screen-bot-setup")));
  chooseLocalBtn.addEventListener("click", () => tryProceedFromLanding(beginLocalGame));
  botBackBtn.addEventListener("click", () => showScreen("screen-landing"));
  botStartBtn.addEventListener("click", beginBotGame);
  newGameBtn.addEventListener("click", () => showScreen("screen-landing"));

  statusEl.textContent = "";
  showScreen("screen-landing");
}

main();
