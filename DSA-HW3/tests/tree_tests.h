#ifndef TREE_TESTS_H
#define TREE_TESTS_H

#include <munit.h>
#include "tree.h"
#include "connect4.h"

static MunitResult test_init_node(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(7, 6);
    TreeNode *node = init_node(state);

    munit_assert_not_null(node);
    munit_assert_int(node->num_children, ==, -1);
    munit_assert_ptr_equal(node->game_state, state);
    munit_assert_null(node->children);

    free_tree(node);
    return MUNIT_OK;
}

static MunitResult test_tree_init(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(3, 4);

    // Test depth 2 tree
    TreeNode *tree = init_tree(state, 2);
    munit_assert_not_null(tree);
    munit_assert_int(tree->num_children, ==, 3); // All columns available
    munit_assert_int(node_count(tree), ==, 4);   // Root + 3 children

    // compare board of root node to initial state
    munit_assert_memory_equal(state->width * state->height, state->board, tree->game_state->board);

    // compare board of root node's children to initial state
    char board[13];
    int row = 3;
    for (int col = 0; col < 3; col++)
    {
        strcpy(board, "____________");
        board[row * state->width + col] = 'X';
        munit_assert_memory_equal(state->width * state->height, board, tree->children[col]->game_state->board);
    }

    munit_assert_int(tree->children[0]->num_children, ==, -1);
    munit_assert_null(tree->children[0]->children);

    free_tree(tree);

    // Test depth 3 tree
    state = init_game_state(7, 6);
    tree = init_tree(state, 3);
    munit_assert_not_null(tree);
    munit_assert_int(tree->num_children, ==, 7);
    munit_assert_int(node_count(tree), ==, 57); // Root + 7 children + 49 grandchildren
    free_tree(tree);

    return MUNIT_OK;
}

static MunitResult test_tree_init_partial_board(const MunitParameter params[], void *data)
{
    int width = 4, height = 4;
    GameState *state = init_game_state(width, height);

    // Fill first column
    for (int i = 0; i < height; i++)
    {
        state->board[i * width + 0] = (i % 2) ? 'O' : 'X';
    }

    TreeNode *tree = init_tree(state, 2);
    munit_assert_not_null(tree);
    munit_assert_int(tree->num_children, ==, 3); // First column full
    munit_assert_int(node_count(tree), ==, 4);   // Root + 3 children

    char board[17]; // 16 + 1 for null terminator
    int row = 3;
    for (int i = 0; i < 3; i++)
    {
        strcpy(board, "________________");
        for (int j = 0; j < height; j++)
        {
            board[j * width + 0] = (j % 2) ? 'O' : 'X';
        }
        board[row * width + i + 1] = 'X';
        munit_assert_memory_equal(width * height, board, tree->children[i]->game_state->board);
    }

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_tree_expand_from_init(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(3, 4);
    TreeNode *tree = init_tree(state, 2);

    int initial_count = node_count(tree);
    expand_tree(tree);
    int expanded_count = node_count(tree);

    munit_assert_int(initial_count, ==, 4);   // 1 + 3
    munit_assert_int(expanded_count, ==, 13); // 1 + 3 + 9

    // Check children's expansion
    for (int i = 0; i < 3; i++)
    {
        munit_assert_int(tree->children[i]->num_children, ==, 3);
        for (int j = 0; j < 3; j++)
        {
            munit_assert_int(tree->children[i]->children[j]->num_children, ==, -1);
        }
    }

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_tree_init_nearly_full(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(7, 3);

    // Fill all but one space
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            if (!(i == 0 && j == 3))
            { // Leave top middle empty
                state->board[i * 7 + j] = (i + j) % 2 ? 'X' : 'O';
            }
        }
    }

    TreeNode *tree = init_tree(state, 2);
    munit_assert_not_null(tree);
    munit_assert_int(tree->num_children, ==, 1); // Only one move possible
    munit_assert_int(node_count(tree), ==, 2);   // Root + 1 child

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_tree_expand_partial(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(7, 6);

    // Fill first column
    for (int i = 0; i < 6; i++)
    {
        state->board[i * 7 + 0] = (i % 2) ? 'O' : 'X';
    }

    TreeNode *tree = init_tree(state, 2);
    int initial_count = node_count(tree);
    expand_tree(tree);
    int expanded_count = node_count(tree);

    munit_assert_int(initial_count, ==, 7);   // Root + 6 possible moves
    munit_assert_int(expanded_count, ==, 43); // Root + 6 children + 36 grandchildren

    free_tree(tree);
    return MUNIT_OK;
}

// init to much depth
static MunitResult test_tree_init_too_deep(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(2, 2);

    TreeNode *tree = init_tree(state, 15); // Should not crash even though possible depth is 5
    munit_assert_not_null(tree);
    munit_assert_int(node_count(tree), ==, 19); // All possible nodes


    free_tree(tree);
    return MUNIT_OK;
}

// Expand all the way to the bottom
static MunitResult test_tree_expand_all_the_way(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(2, 2);
    TreeNode *tree = init_tree(state, 2);
    munit_assert_int(node_count(tree), ==, 3);

    expand_tree(tree);
    munit_assert_int(node_count(tree), ==, 7);

    expand_tree(tree);
    munit_assert_int(node_count(tree), ==, 13);

    expand_tree(tree);
    munit_assert_int(node_count(tree), ==, 19);

    free_tree(tree);
    return MUNIT_OK;
}

// expand too deep
static MunitResult test_tree_expand_too_deep(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(2, 2);
    TreeNode *tree = init_tree(state, 15);
    munit_assert_int(node_count(tree), ==, 19);

    expand_tree(tree);
    munit_assert_int(node_count(tree), ==, 19);

    expand_tree(tree);
    munit_assert_int(node_count(tree), ==, 19);

    free_tree(tree);
    return MUNIT_OK;
}

static MunitResult test_tree_no_child_for_finished_game(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(4, 4);
    state->board[0 * 4 + 0] = 'X';
    state->board[1 * 4 + 0] = 'X';
    state->board[2 * 4 + 0] = 'X';
    state->board[3 * 4 + 0] = 'X';

    state->board[3 * 4 + 2] = 'O';
    state->board[3 * 4 + 3] = 'O';
    state->board[2 * 4 + 3] = 'O';
    state->next_turn = true;

    TreeNode *tree = init_tree(state, 4);
    munit_assert_int(tree->num_children, ==, -1);
    munit_assert_null(tree->children);
    munit_assert_int(node_count(tree), ==, 1);

    free_tree(tree);
    return MUNIT_OK;
}

// No expand for finished game state
static MunitResult test_tree_no_expand_for_finished_game(const MunitParameter params[], void *data)
{
    GameState *state = init_game_state(4, 4);
    state->board[0 * 4 + 0] = 'X';
    state->board[1 * 4 + 0] = 'X';
    state->board[2 * 4 + 0] = 'X';
    state->board[3 * 4 + 0] = 'X';

    state->board[3 * 4 + 2] = 'O';
    state->board[3 * 4 + 3] = 'O';
    state->board[2 * 4 + 3] = 'O';
    state->next_turn = true;

    TreeNode *tree = init_tree(state, 4);
    expand_tree(tree);
    munit_assert_int(tree->num_children, ==, -1);
    munit_assert_null(tree->children);
    munit_assert_int(node_count(tree), ==, 1);

    free_tree(tree);
    return MUNIT_OK;
}

// Test suite array
static MunitTest tree_tests[] = {
    {"/init_node", test_init_node, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/init_tree", test_tree_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/expand_from_init", test_tree_expand_from_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/init_partial_board", test_tree_init_partial_board, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/init_nearly_full", test_tree_init_nearly_full, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/expand_partial", test_tree_expand_partial, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/init_too_deep", test_tree_init_too_deep, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/expand_all_the_way", test_tree_expand_all_the_way, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/expand_too_deep", test_tree_expand_too_deep, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/no_child_for_finished_game", test_tree_no_child_for_finished_game, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/no_expand_for_finished_game", test_tree_no_expand_for_finished_game, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

#endif // TREE_TESTS_H