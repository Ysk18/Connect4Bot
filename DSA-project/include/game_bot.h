#ifndef GAME_BOT_H
#define GAME_BOT_H

#include "connect4.h"
#include "tree.h"

// First calls the eval_game_tree function then returns the best move for the player using minimax algorithm
// Returns the index of the best move among the children of the root, not among the all moves
int best_move(TreeNode* root);

// Part of the minimax algorithm
// (Do not call this function directly, it supposed to be called by best_move function)
int get_max(TreeNode* node);

// Part of the minimax algorithm
// (Do not call this function directly, it supposed to be called by best_move function)
int get_min(TreeNode* node);

// Returns the evaluation for the given game state
// Assigns 1000 if player 1 wins currently, -1000 if player 2 wins currently, 0 if draw, and a value between -1000 and 1000 otherwise
// Already implemented for you, do not change it
void eval_game_state(GameState *gs); 

// Given a root node, evaluate all the leaf nodes using eval_game_state function
void eval_game_tree(TreeNode *root);

#endif