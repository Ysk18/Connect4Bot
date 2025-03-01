#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "interface.h"
#include "game_bot.h"
#include "tree.h"

// Given a game state, return a tree node
TreeNode *init_node(GameState *gs)
{
    TreeNode *root = (TreeNode *)malloc(sizeof(TreeNode));
    root->game_state = gs;
    root->num_children = -1; // get min max ta erişebilmek için initial -1 ata
    root->children = NULL;
    return root;
}

// Given a game state, construct the tree up to the given depth using the available moves for each node
// Returns the root node
// You can assume that depth >= 2
TreeNode *init_tree(GameState *gs, int depth)
{
    TreeNode *root = init_node(gs);
    init_tree_2(&root, depth - 1);
    return root;
}

void init_tree_2(TreeNode **root, int depth)
{
    // print_game_state(gs);
    if (depth == 0 || get_game_status((*root)->game_state) != IN_PROGRESS)
        return;

    bool moves[(*root)->game_state->width];
    memset(moves, false, (*root)->game_state->width * sizeof(bool));
    int possible_move_count = available_moves((*root)->game_state, moves);

    (*root)->num_children = possible_move_count;
    (*root)->children = (TreeNode **)malloc(sizeof(TreeNode *) * possible_move_count);

    int index = 0;
    for (int i = 0; i < possible_move_count; i++)
    {
        // make move then add to the children
        while (moves[index] != true) // to find new state
            index++;

        TreeNode *add = init_node(make_move((*root)->game_state, index));
        (*root)->children[i] = add;

        init_tree_2(&((*root)->children[i]), depth - 1);
        index++;
    }
}

// Frees the tree
void free_tree(TreeNode *root) // su an hatali
{
    if (!root)
        return;
    for (int i = 0; i < root->num_children; i++)
        free_tree(root->children[i]);
    free_node(root);
}
void free_node(TreeNode *node) // su an hatali
{
    if (!node)
        return;
    if (node->game_state)
        free_game_state(node->game_state);
    if (node->children)
    {
        free(node->children);
        node->children = NULL;
    }
    free(node);
    node = NULL;
}

// Expand all leaf nodes of the tree by one level
void expand_tree(TreeNode *root)
{
    if (!root)
        return;
    if (!root->children)
        init_tree_2(&root, 1);
    else
        for (int i = 0; i < root->num_children; i++)
            expand_tree(root->children[i]);
}

// Count the number of nodes in the tree
int node_count(TreeNode *root)
{
    if (!root)
        return 0;
    if (root->num_children == -1 || !root->children)
        return 1;
    int count = 1;
    for (int i = 0; i < root->num_children; i++)
    {
        count += node_count(root->children[i]);
    }
    return count;
}

// Print the tree for debugging purposes (optional)
void print_tree(TreeNode *root, int level)
{
    if (!root || level < 0)
        return;
    if (root->children)
    {

        printf("Level 0: ");
        for (int i = 0; i < root->num_children; i++)
        {
            printf("- (%d) ", get_min(root->children[i]));
            // printf("- (%d) ", root->children[i]->game_state->evaluation); // debug
        }
        printf(" -\n");
        /*
        printf("Level 1: ");
        for (int i = 0; i < root->num_children; i++)
        {
            printf("\n--");
            if (root->children[i]->children == NULL)
            {
                printf("Game ending situation...");
                continue;
            }
            for (int j = 0; j < root->num_children; j++)
            {
                root->children[i]->children[j]->game_state->evaluation = get_max(root->children[i]);
                printf("- (%d) ", root->children[i]->children[j]->game_state->evaluation);
            }
        }
        printf("\n");
        */
    }
}
