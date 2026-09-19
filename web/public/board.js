// Pure rendering module: draws the board as a DOM grid and wires column clicks.
// Kept free of any WASM/game-state knowledge beyond the plain data it's given.

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

// state: { width, height, getCell(row, col) -> 'X'|'O'|'_', isColumnFull(col) -> bool }
export function renderBoard(columns, state) {
  for (let col = 0; col < state.width; col++) {
    const { el, cells } = columns[col];
    el.classList.toggle("column-full", state.isColumnFull(col));

    for (let row = 0; row < state.height; row++) {
      const value = state.getCell(row, col);
      const cellEl = cells[row];
      cellEl.classList.remove("x", "o");
      if (value === "X") cellEl.classList.add("x");
      else if (value === "O") cellEl.classList.add("o");
    }
  }
}
