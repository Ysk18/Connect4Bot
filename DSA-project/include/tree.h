#ifndef TREE_H
#define TREE_H

#include "connect4.h"

// A node for generic tree
typedef struct TreeNode
{
    int num_children; // Number of children (-1 if it is not yet known, 0 if it is a terminal node i.e. board is full)
    GameState *game_state;
    struct TreeNode **children; // Array of child nodes
} TreeNode;

// Given a game state, return a tree node
TreeNode *init_node(GameState *gs);

// Given a game state, construct the tree up to the given depth using the available moves for each node
// Returns the root node
// You can assume that depth >= 2
TreeNode *init_tree(GameState *gs, int depth);
void init_tree_2(TreeNode **root, int depth);

// Frees the tree
void free_tree(TreeNode *root);
void free_node(TreeNode *node);

// Expand all leaf nodes of the tree by one level
void expand_tree(TreeNode *root);

// Count the number of nodes in the tree
int node_count(TreeNode *root);

// Print the tree for debugging purposes (optional)
void print_tree(TreeNode *root, int level);

#endif