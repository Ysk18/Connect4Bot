// DO NOT MODIFY eval_game_state FUNCTION
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "connect4.h"
#include "game_bot.h"

void eval_game_state(GameState *gs)
{
    GameStatus status = get_game_status(gs);
    if (status == PLAYER_1_WIN)
    {
        gs->evaluation = 1000;
        return;
    }
    else if (status == PLAYER_2_WIN)
    {
        gs->evaluation = -1000;
        return;
    }
    else if (status == DRAW)
    {
        gs->evaluation = 0;
        return;
    }

    // Count the number of 3s in a row
    int player_1_3 = 0;
    int player_2_3 = 0;

    // Count the number of 2s in a row with an extra space around
    int player_1_2 = 0;
    int player_2_2 = 0;

    // Check horizontal
    for (int i = 0; i < gs->height; i++)
    {
        for (int j = 0; j <= gs->width - 3; j++)
        {
            int index = i * gs->width + j;

            int x_count = (gs->board[index] == 'X') + (gs->board[index + 1] == 'X') + (gs->board[index + 2] == 'X');
            int o_count = (gs->board[index] == 'O') + (gs->board[index + 1] == 'O') + (gs->board[index + 2] == 'O');
            int empty_count = (gs->board[index] == '_') + (gs->board[index + 1] == '_') + (gs->board[index + 2] == '_');

            if (x_count == 3)
                player_1_3++;
            else if (o_count == 3)
                player_2_3++;
            else if (x_count == 2 && empty_count == 1)
                player_1_2++;
            else if (o_count == 2 && empty_count == 1)
                player_2_2++;
        }
    }

    // Check vertical
    for (int i = 0; i <= gs->height - 3; i++)
    {
        for (int j = 0; j < gs->width; j++)
        {
            int index = i * gs->width + j;
            // if (gs->board[index] != '_' &&
            //     gs->board[index] == gs->board[index + gs->width] &&
            //     gs->board[index] == gs->board[index + 2 * gs->width])
            // {
            //     if (gs->board[index] == 'X')
            //         player_1_3++;
            //     else
            //         player_2_3++;
            // }

            int x_count = (gs->board[index] == 'X') + (gs->board[index + gs->width] == 'X') + (gs->board[index + 2 * gs->width] == 'X');
            int o_count = (gs->board[index] == 'O') + (gs->board[index + gs->width] == 'O') + (gs->board[index + 2 * gs->width] == 'O');
            int empty_count = (gs->board[index] == '_') + (gs->board[index + gs->width] == '_') + (gs->board[index + 2 * gs->width] == '_');

            if (x_count == 3)
                player_1_3++;
            else if (o_count == 3)
                player_2_3++;
            else if (x_count == 2 && empty_count == 1)
                player_1_2++;
            else if (o_count == 2 && empty_count == 1)
                player_2_2++;
        }
    }

    // Check diagonal (top-left to bottom-right)
    for (int i = 0; i <= gs->height - 3; i++)
    {
        for (int j = 0; j <= gs->width - 3; j++)
        {
            int index = i * gs->width + j;
            // if (gs->board[index] != '_' &&
            //     gs->board[index] == gs->board[index + gs->width + 1] &&
            //     gs->board[index] == gs->board[index + 2 * gs->width + 2])
            // {
            //     if (gs->board[index] == 'X')
            //         player_1_3++;
            //     else
            //         player_2_3++;
            // }

            int x_count = (gs->board[index] == 'X') + (gs->board[index + gs->width + 1] == 'X') + (gs->board[index + 2 * gs->width + 2] == 'X');
            int o_count = (gs->board[index] == 'O') + (gs->board[index + gs->width + 1] == 'O') + (gs->board[index + 2 * gs->width + 2] == 'O');
            int empty_count = (gs->board[index] == '_') + (gs->board[index + gs->width + 1] == '_') + (gs->board[index + 2 * gs->width + 2] == '_');

            if (x_count == 3)
                player_1_3++;
            else if (o_count == 3)
                player_2_3++;
            else if (x_count == 2 && empty_count == 1)
                player_1_2++;
            else if (o_count == 2 && empty_count == 1)
                player_2_2++;
        }
    }

    // Check diagonal (top-right to bottom-left)
    for (int i = 0; i <= gs->height - 4; i++)
    {
        for (int j = gs->width - 1; j >= 2; j--)
        {
            int index = i * gs->width + j;
            // if (gs->board[index] != '_' &&
            //     gs->board[index] == gs->board[index + gs->width - 1] &&
            //     gs->board[index] == gs->board[index + 2 * gs->width - 2])
            // {
            //     if (gs->board[index] == 'X')
            //         player_1_3++;
            //     else
            //         player_2_3++;
            // }

            int x_count = (gs->board[index] == 'X') + (gs->board[index + gs->width - 1] == 'X') + (gs->board[index + 2 * gs->width - 2] == 'X');
            int o_count = (gs->board[index] == 'O') + (gs->board[index + gs->width - 1] == 'O') + (gs->board[index + 2 * gs->width - 2] == 'O');
            int empty_count = (gs->board[index] == '_') + (gs->board[index + gs->width - 1] == '_') + (gs->board[index + 2 * gs->width - 2] == '_');

            if (x_count == 3)
                player_1_3++;
            else if (o_count == 3)
                player_2_3++;
            else if (x_count == 2 && empty_count == 1)
                player_1_2++;
            else if (o_count == 2 && empty_count == 1)
                player_2_2++;
        }
    }

    gs->evaluation = 10 * (player_1_3 - player_2_3) + 3 * (player_1_2 - player_2_2);
}

int best_move(TreeNode *root)
{

    if (!root || !root->children)
        return -1;
    eval_game_tree(root);

    int bestMoveEvaluation = -1000;
    int bestMove = 0;
    if (root->game_state->next_turn == false) // 1st player
    {
        for (int i = 0; i < root->num_children; i++)
        {
            int max_gain = get_min(root->children[i]);
            if (max_gain > bestMoveEvaluation)
            {
                bestMoveEvaluation = max_gain;
                bestMove = i;
            }
        }
    }
    else // 2nd player
    {
        bestMoveEvaluation = 1000;
        for (int i = 0; i < root->num_children; i++)
        {
            int max_gain = get_max(root->children[i]);
            if (max_gain < bestMoveEvaluation)
            {
                bestMoveEvaluation = max_gain;
                bestMove = i;
            }
        }
    }

    // printf("Best move evaluation: %d at index (index of chidren): %d.\n", bestMoveEvaluation, bestMove); // debug
    // we have best move evaluation
    // root->game_state->evaluation = bestMoveEvaluation; // debug
    return bestMove;
}

// Part of the minimax algorithm
// (Do not call this function directly, it supposed to be called by best_move function)
int get_max(TreeNode *node)
{
    if (!node)
        return 0;

    int max_gain = -1000; // [-1000, 1000]
    if (!node->children)
        return node->game_state->evaluation;

    else
    {
        for (int i = 0; i < node->num_children; i++)
        {
            int min_result = get_min(node->children[i]);
            if (min_result > max_gain)
                max_gain = min_result;
        }
    }
    // node->game_state->evaluation = max_gain; // debug
    return max_gain;
}

// Part of the minimax algorithm
// (Do not call this function directly, it supposed to be called by best_move function)
int get_min(TreeNode *node)
{
    if (!node)
        return 0;

    int min_gain = 1000; // [-1000, 1000]
    if (!node->children)
        return node->game_state->evaluation;

    else
    {
        for (int i = 0; i < node->num_children; i++)
        {
            int max_result = get_max(node->children[i]);
            if (i == 0)
                min_gain = max_result;
            else if (max_result < min_gain)
                min_gain = max_result;
        }
    }

    // node->game_state->evaluation = min_gain; // debug
    return min_gain;
}

// Given a root node, evaluate all the leaf nodes using eval_game_state function
void eval_game_tree(TreeNode *root)
{
    if (!root)
        return;
    if (!root->children)
    {
        eval_game_state(root->game_state);
        return;
    }

    for (int i = 0; i < root->num_children; i++)
        eval_game_tree(root->children[i]);
}