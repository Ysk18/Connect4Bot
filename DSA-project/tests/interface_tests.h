#ifndef INTERFACE_TESTS_H
#define INTERFACE_TESTS_H

#include <munit.h>
#include "interface.h"

static MunitResult test_apply_move_to_tree1(const MunitParameter params[], void *data)
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

    TreeNode *root = init_tree(state, 10);
    int move = 1; // Remember! this move is indexed among possible moves, not the actual board position
    TreeNode *next_root = root->children[move];

    // Test applying a move
    int initial_count = node_count(root);
    munit_assert_char(root->game_state->board[1 * width + 3], ==, '_'); 

    apply_move_to_tree(&root, move, 10);
    int final_count = node_count(root);

    // Check that tree was pruned
    munit_assert_ptr_equal(root, next_root);
    munit_assert_int(initial_count, ==, 16);
    munit_assert_int(final_count, ==, 6);

    // Check that move was applied correctly
    munit_assert_char(root->game_state->board[1 * width + 3], ==, 'X');

    free_tree(root);
    return MUNIT_OK;
}


static MunitResult test_play_game(const MunitParameter params[], void *data)
{
    // This test is not that really important
    // If you want to master the C, try to understand this test.
    // Basically, we want to test the play_game function, but we want to do it in a controlled way.
    // So we redirect stdout to a temp file, and then we read the last line of the temp file to check the output.
    // We also want to test the function with simulated human input, so we redirect stdin to a simulated input.

    // Create a temp file to store stdout
    FILE *temp_stdout = tmpfile();
    // Redirect stdout to the temp file
    stdout = temp_stdout;

    // Play the game with simulated human input
    char input[] = "0\n1\n0\n1\n";
    stdin = fmemopen(input, 9, "r");

    play_game(5, 5, 6, true);

    // Read the last 14 characters of the temp file
    char buffer[14];
    fseek(temp_stdout, -14, SEEK_END);
    fread(buffer, 1, 14, temp_stdout);
    buffer[13] = '\0';
    munit_assert_string_equal(buffer, "Player 1 won!");

    return MUNIT_OK;
}


// Test suite array
static MunitTest interface_tests[] = {
    {"/apply_move_to_tree", test_apply_move_to_tree1, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/play_game", test_play_game, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

#endif // INTERFACE_TESTS_H