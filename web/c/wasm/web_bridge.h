#ifndef WEB_BRIDGE_H
#define WEB_BRIDGE_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Lifecycle */
EMSCRIPTEN_KEEPALIVE
void wb_start_game(int width, int height, int tree_depth);

EMSCRIPTEN_KEEPALIVE
void wb_reset_game(void); /* reuses the width/height/tree_depth from the last wb_start_game call */

/* Board and state queries, for rendering */
EMSCRIPTEN_KEEPALIVE
int wb_get_width(void);

EMSCRIPTEN_KEEPALIVE
int wb_get_height(void);

/* Returns 'X' / 'O' / '_' for cell (row, col). row 0 = top, matches GameState->board layout. */
EMSCRIPTEN_KEEPALIVE
char wb_get_cell(int row, int col);

/* 0 = player 1 to move ('X'), 1 = player 2 to move ('O') */
EMSCRIPTEN_KEEPALIVE
int wb_get_next_turn(void);

/* Maps GameStatus enum: 0=PLAYER_1_WIN, 1=PLAYER_2_WIN, 2=DRAW, 3=IN_PROGRESS */
EMSCRIPTEN_KEEPALIVE
int wb_get_status(void);

/* Legal moves */
EMSCRIPTEN_KEEPALIVE
int wb_is_move_available(int column); /* 1 if column is currently legal, else 0 */

/* Applies a move for whoever's turn it currently is. column is a RAW board column
   (0..width-1), NOT a child index; the wrapper does the column -> child-index
   translation internally. Returns the new status after the move, or -1 if the
   column was illegal (no state change). Usable for both human clicks and for
   replaying the bot's chosen column on a mirror instance (see frontend design). */
EMSCRIPTEN_KEEPALIVE
int wb_do_human_move(int column);

/* Alias of wb_do_human_move, for call sites where the mover isn't a human
   (e.g. replaying the bot's move on the main-thread mirror instance). */
EMSCRIPTEN_KEEPALIVE
int wb_apply_move(int column);

/* Bot move: synchronous, runs best_move() on the current tree and applies it.
   Meant to be invoked from inside a Web Worker so it never blocks the UI thread.
   Returns the RAW column the bot played (0..width-1), or -1 on error/no legal moves. */
EMSCRIPTEN_KEEPALIVE
int wb_do_bot_move(void);

/* Diagnostics, useful for tuning depth and for regression-testing tree growth */
EMSCRIPTEN_KEEPALIVE
int wb_get_node_count(void);

#ifdef __cplusplus
}
#endif
#endif
