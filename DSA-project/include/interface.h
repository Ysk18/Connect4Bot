#ifndef INTERFACE_H
#define INTERFACE_H

#include "connect4.h"
#include "tree.h"
#include "game_bot.h"

// Prints the available moves, and gets the human move among the children, not the all moves
int get_human_move(GameState *gs);

// Returns the index of the random move among the children, not the all moves
// DO NOT CALL THIS FUNCTION UNNECESSARILY, IT WILL BREAK REPRODUCIBILITY 
// (assumes that there is at least one available move)
int get_random_move(GameState *gs);

// Step function for the game loop:
// - Update the root according to "move"
// - Prune unnecessary subtrees
// - Expand the tree one more depth
// - if node count is less than game_width^(depth-2), then expand the tree one more depth
void apply_move_to_tree(TreeNode **root, int move, int initial_tree_depth);

// Game loop
void play_game(int game_width, int game_height, int tree_depth, bool play_against_human);

#endif