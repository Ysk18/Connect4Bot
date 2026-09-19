// New file, not part of the graded DSA-project sources.
// Thin bridge between the untouched Connect4 engine (DSA-project/include, DSA-project/src)
// and a browser frontend compiled via Emscripten. Owns exactly one live game session
// (a single TreeNode tree) as static globals inside the long-lived WASM instance.
//
// The reroot-on-move logic mirrors DSA-project/src/interface.c's apply_move_to_tree,
// and the raw-column <-> child-index translation mirrors interface.c's get_human_move.

#include "web_bridge.h"
#include "connect4.h"
#include "tree.h"
#include "game_bot.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

static TreeNode *g_root = NULL;
static int g_width = 0;
static int g_height = 0;
static int g_tree_depth = 0;

// Applies the move at the given child index (index among g_root's children, NOT a raw
// column) by rerooting the tree at that child and freeing every sibling subtree, then
// tops the tree back up to depth. Mirrors interface.c's apply_move_to_tree exactly.
static void reroot_and_expand(int child_index)
{
    if (!g_root)
        return;

    TreeNode *new_root = NULL;
    for (int i = 0; i < g_root->num_children; i++)
    {
        if (i == child_index)
            new_root = g_root->children[i];
        else
            free_tree(g_root->children[i]);
    }
    free_node(g_root);
    g_root = new_root;

    expand_tree(g_root);
    if (node_count(g_root) < pow(g_root->game_state->width, (g_tree_depth - 2)))
        expand_tree(g_root);
}

// Translates a raw board column into "index among currently available children",
// exactly like interface.c's get_human_move. Returns -1 if the column is out of
// range or not currently legal.
static int column_to_child_index(int column)
{
    if (column < 0 || column >= g_width)
        return -1;

    bool full_moves[64]; // g_width is small (board widths are tiny); 64 is a generous cap
    if (g_width > 64)
        return -1;
    memset(full_moves, 0, sizeof(full_moves));
    available_moves(g_root->game_state, full_moves);

    if (!full_moves[column])
        return -1;

    int move_among_children = 0;
    for (int i = 0; i <= column; i++)
    {
        if (full_moves[i])
            move_among_children++;
    }
    return move_among_children - 1;
}

// Recovers the raw board column corresponding to a given "index among children"
// (the inverse of column_to_child_index), used to report which column the bot chose.
static int child_index_to_column(int child_index)
{
    bool full_moves[64];
    if (g_width > 64)
        return -1;
    memset(full_moves, 0, sizeof(full_moves));
    available_moves(g_root->game_state, full_moves);

    int count = -1;
    for (int col = 0; col < g_width; col++)
    {
        if (full_moves[col])
        {
            count++;
            if (count == child_index)
                return col;
        }
    }
    return -1;
}

void wb_start_game(int width, int height, int tree_depth)
{
    if (g_root)
    {
        free_tree(g_root);
        g_root = NULL;
    }

    g_width = width;
    g_height = height;
    g_tree_depth = tree_depth;

    GameState *gs = init_game_state(width, height);
    g_root = init_tree(gs, tree_depth); // init_tree takes ownership of gs
}

void wb_reset_game(void)
{
    wb_start_game(g_width, g_height, g_tree_depth);
}

int wb_get_width(void)
{
    return g_width;
}

int wb_get_height(void)
{
    return g_height;
}

char wb_get_cell(int row, int col)
{
    if (!g_root || row < 0 || row >= g_height || col < 0 || col >= g_width)
        return '_';
    return g_root->game_state->board[row * g_width + col];
}

int wb_get_next_turn(void)
{
    if (!g_root)
        return 0;
    return g_root->game_state->next_turn ? 1 : 0;
}

int wb_get_status(void)
{
    if (!g_root)
        return IN_PROGRESS;
    return (int)get_game_status(g_root->game_state);
}

int wb_is_move_available(int column)
{
    if (!g_root || column < 0 || column >= g_width || g_width > 64)
        return 0;

    bool full_moves[64];
    memset(full_moves, 0, sizeof(full_moves));
    available_moves(g_root->game_state, full_moves);
    return full_moves[column] ? 1 : 0;
}

int wb_do_human_move(int column)
{
    if (!g_root || get_game_status(g_root->game_state) != IN_PROGRESS)
        return -1;

    int child_index = column_to_child_index(column);
    if (child_index < 0)
        return -1;

    reroot_and_expand(child_index);
    return wb_get_status();
}

int wb_apply_move(int column)
{
    return wb_do_human_move(column);
}

int wb_do_bot_move(void)
{
    if (!g_root || get_game_status(g_root->game_state) != IN_PROGRESS)
        return -1;

    int child_index = best_move(g_root);
    if (child_index < 0)
        return -1;

    int column = child_index_to_column(child_index);
    if (column < 0)
        return -1;

    reroot_and_expand(child_index);
    return column;
}

int wb_get_node_count(void)
{
    if (!g_root)
        return 0;
    return node_count(g_root);
}
