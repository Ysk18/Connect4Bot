// Rendering module: draws the board as a DOM grid, wires column clicks, and
// animates a newly dropped piece falling from above the board with a small
// landing bounce. Kept free of any WASM/game-state knowledge beyond the plain
// data and coordinates it's given.

export function buildBoard(container, width, height, onColumnClick) {
  container.innerHTML = "";
  container.style.gridTemplateColumns = `repeat(${width}, 1fr)`;

  const columns = [];
  for (let col = 0; col < width; col++) {
    const columnEl = document.createElement("div");
    columnEl.className = "column";
    columnEl.dataset.col = String(col);
    columnEl.addEventListener("click", () => onColumnClick(col));

    const cells = [];
    for (let row = 0; row < height; row++) {
      const cellEl = document.createElement("div");
      cellEl.className = "cell";
      columnEl.appendChild(cellEl);
      cells.push(cellEl);
    }

    container.appendChild(columnEl);
    columns.push({ el: columnEl, cells });
  }

  return columns;
}

function setCellValue(columns, row, col, value) {
  const cellEl = columns[col].cells[row];
  cellEl.classList.remove("x", "o", "dropping");
  if (value === "X") cellEl.classList.add("x");
  else if (value === "O") cellEl.classList.add("o");
}

// Full static repaint (no animation) -- used when starting/resetting a game.
// state: { width, height, getCell(row, col) -> 'X'|'O'|'_', isColumnFull(col) -> bool }
export function renderBoard(columns, state) {
  for (let col = 0; col < state.width; col++) {
    columns[col].el.classList.toggle("column-full", state.isColumnFull(col));
    for (let row = 0; row < state.height; row++) {
      setCellValue(columns, row, col, state.getCell(row, col));
    }
  }
}

export function updateColumnFullStates(columns, width, isColumnFull) {
  for (let col = 0; col < width; col++) {
    columns[col].el.classList.toggle("column-full", isColumnFull(col));
  }
}

// Animates the piece that was just placed at (row, col) falling from above
// the board down into its slot, with a small bounce on impact.
//
// Returns { duration, whenLanded }:
//   - duration: the animation length in seconds (for timing a landing sound).
//   - whenLanded: a promise that resolves once the fall animation has ACTUALLY
//     finished playing in the browser (via the real `animationend` event, not
//     a guessed timer), so callers can reliably sequence something to happen
//     only after this piece is fully settled. A short fallback timeout backs
//     it up in case `animationend` never fires (e.g. animations disabled).
export function dropPiece(columns, row, col, value) {
  const cellEl = columns[col].cells[row];
  const columnEl = columns[col].el;

  const cellRect = cellEl.getBoundingClientRect();
  const columnStyles = getComputedStyle(columnEl);
  const gap = parseFloat(columnStyles.rowGap || columnStyles.gap) || 0;
  const step = cellRect.height + gap;

  // Distance from above the top of the column down to this row's slot.
  const fallDistance = (row + 1) * step;
  // Heavier-feeling fall for pieces that travel further, capped so it never
  // feels sluggish on a tall board.
  const duration = Math.min(0.9, 0.32 + row * 0.045);

  cellEl.style.setProperty("--fall-distance", `${-fallDistance}px`);
  cellEl.style.setProperty("--drop-duration", `${duration}s`);

  cellEl.classList.remove("x", "o", "dropping");
  void cellEl.offsetWidth; // force reflow so the animation restarts every time
  if (value === "X") cellEl.classList.add("x");
  else if (value === "O") cellEl.classList.add("o");
  cellEl.classList.add("dropping");

  const whenLanded = new Promise((resolve) => {
    let settled = false;
    const finish = () => {
      if (settled) return;
      settled = true;
      cellEl.removeEventListener("animationend", finish);
      resolve();
    };
    cellEl.addEventListener("animationend", finish, { once: true });
    setTimeout(finish, duration * 1000 + 100); // safety net, real event should win
  });

  return { duration, whenLanded };
}
