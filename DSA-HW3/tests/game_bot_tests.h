#ifndef GAME_BOT_TESTS_H
#define GAME_BOT_TESTS_H

#include <munit.h>
#include "game_bot.h"

static MunitResult test_eval_game_tree(const MunitParameter params[], void *data)
{
    int width = 3, height = 4;
    GameState *state = init_game_state(width, height);

    // fill the first column alternately
    for (int i = 0; i < height; i++)
    {
        state->board[i * width + 0] = i % 2 == 0 ? 'X' : 'O';
    }

    // fill the first row alternately
    for (int i = 0; i < width; i++)
    {
        state->board[0 * width + i] = i % 2 == 0 ? 'X' : 'O';
    }

    TreeNode *tree = init_tree(state, 3);

    eval_game_tree(tree);

    // Test that all leaf nodes have been evaluated
    for (int i = 0; i < tree->num_children; i++)
    {
        for (int j = 0; j < tree->children[i]->num_children; j++)
        {
            GameState *leaf_gs = tree->children[i]->children[j]->game_state;

            GameState *leaf_gs_copy = (GameState *)malloc(sizeof(GameState));
            memcpy(leaf_gs_copy, leaf_gs, sizeof(GameState));
            leaf_gs_copy->board = (char *)malloc(leaf_gs->height * leaf_gs->width * sizeof(char));
            memcpy(leaf_gs_copy->board, leaf_gs->board, leaf_gs->height * leaf_gs->width * sizeof(char));

            eval_game_state(leaf_gs_copy);
            munit_assert_int(leaf_gs_copy->evaluation, ==, leaf_gs->evaluation);

            free_game_state(leaf_gs_copy);
        }
    }

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_best_move_trivial(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(7, 6);

    // Create a winning opportunity for player 1
    for (int i = 0; i < 3; i++)
    {
        state->board[5 * 7 + i] = 'X';
    }
    // Board:
    // _ _ _ _ _ _ _
    // _ _ _ _ _ _ _
    // _ _ _ _ _ _ _
    // _ _ _ _ _ _ _
    // _ _ _ _ _ _ _
    // X X X _ _ _ _

    TreeNode *tree = init_tree(state, 3);

    int move = best_move(tree);

    munit_assert_int(move, ==, 3);

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_best_move_alternating(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(4, 3);

    // Create a winning opportunity for player 1
    state->board[2 * 4 + 0] = 'X';
    state->board[1 * 4 + 0] = 'O';
    state->board[2 * 4 + 1] = 'X';
    state->board[0 * 4 + 0] = 'X';
    // Board:
    // X _ _ _
    // O _ _ _
    // X X _ _

    TreeNode *tree = init_tree(state, 20);

    int move = best_move(tree);

    munit_assert_int(move, ==, 0);

    move = best_move(tree->children[1]); // Lets choose the second move
    // Board:
    // X _ _ _
    // O _ _ _
    // X X X _
    munit_assert_int(move, ==, 2);

    move = best_move(tree->children[2]); // Lets choose the third move
    // Board:
    // X _ _ _
    // O _ _ _
    // X X _ X
    munit_assert_int(move, ==, 1);

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_best_move_nearly_full(const MunitParameter params[], void *data)
{
    int width = 4, height = 4;
    GameState *state = init_game_state(width, height);

    // Create a winning opportunity for player 2
    state->board[3 * width + 0] = 'O';
    state->board[3 * width + 1] = 'X';
    state->board[3 * width + 2] = 'X';
    state->board[3 * width + 3] = 'X';

    state->board[2 * width + 0] = 'O';
    state->board[2 * width + 1] = 'O';
    state->board[2 * width + 2] = 'O';
    state->board[2 * width + 3] = 'X';

    state->board[1 * width + 1] = 'X';
    state->board[1 * width + 2] = 'O';

    state->board[0 * width + 1] = 'X';
    state->board[0 * width + 2] = 'O';
    // Board:
    // _ X O _
    // _ X O _
    // O O O X
    // O X X X

    TreeNode *tree = init_tree(state, 10);

    munit_assert_int(node_count(tree), ==, 16);

    int move = best_move(tree);
    munit_assert_int(move, ==, 0); // Either move is winning, but lower childs are preferred always

    move = best_move(tree->children[1]);
    munit_assert_int(move, ==, 1);

    free_tree(tree);
    return MUNIT_OK;
}

// Test suite array
static MunitTest game_bot_tests[] = {
    {"/eval_game_tree", test_eval_game_tree, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/best_move_trivial", test_best_move_trivial, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/best_move_alternating", test_best_move_alternating, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/best_move_nearly_full", test_best_move_nearly_full, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

#endif // GAME_BOT_TESTS_H