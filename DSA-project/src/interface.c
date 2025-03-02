// ONLY MODIFY THE "apply_move_to_tree" FUNCTION

#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

int get_random_move(GameState *gs)
{
    // This function assumes that there is at least one available move
    bool moves[gs->width];
    memset(moves, false, gs->width * sizeof(bool));
    int move_count = available_moves(gs, moves);

    return rand() % move_count;
}

int get_human_move(GameState *gs)
{
    bool moves[gs->width];
    memset(moves, 0, gs->width * sizeof(bool));
    available_moves(gs, moves);

    printf("Available moves: ");
    for (int i = 0; i < gs->width; i++)
    {
        if (moves[i])
            printf("%d ", i);
    }

    char c;
    printf("\nEnter your move: ");
    c = getchar();
    getchar(); // dump newline character
    int human_move = c - '0';

    // get the index of the move among the children, not the all moves
    int move_among_children = 0;

    for (int i = 0; i <= human_move; i++)
    {
        if (moves[i])
        {
            move_among_children++;
        }
    }

    return move_among_children - 1;
}

void play_game(int game_width, int game_height, int tree_depth, bool play_against_human)
{
    // Init game state
    GameState *state = init_game_state(game_width, game_height);

    // Init tree
    TreeNode *root = init_tree(state, tree_depth);

    int move;

    // While game is not over
    while (get_game_status(root->game_state) == IN_PROGRESS)
    {
        // Get the move
        if (root->game_state->next_turn) // Player 2
        {
            if (play_against_human)
                move = get_human_move(root->game_state);
            else
                move = get_random_move(root->game_state); // move = best_move(root); // debug
        }
        else // Player 1
        {
            move = best_move(root);
        }
        // print_tree(root, 3); // debug
        // Apply the move
        apply_move_to_tree(&root, move, tree_depth);

        // Print the board
        printf("Node Count: %d\n", node_count(root));

        // printf("Move index will be showed: %d\n", move); // debug

        print_game_state(root->game_state);
    }

    // Declare the winner
    if (get_game_status(root->game_state) == DRAW)
        printf("It is a draw\n");
    else
        printf("Player %d won!\n", get_game_status(root->game_state) + 1);

    free_tree(root);
}

void apply_move_to_tree(TreeNode **root, int move, int initial_tree_depth) // ??????????????
{
    if (!(*root))
        return;

    TreeNode *newRoot = NULL;
    for (int i = 0; i < (*root)->num_children; i++)
    {
        if (i == move) // new root, free the parent after for loop
            newRoot = (*root)->children[i];
        else // destruct other trees
            free_tree((*root)->children[i]);
    }
    free_node((*root));
    (*root) = newRoot;

    expand_tree((*root));
    if (node_count((*root)) < pow((*root)->game_state->width, (initial_tree_depth - 2)))
        expand_tree((*root));
}