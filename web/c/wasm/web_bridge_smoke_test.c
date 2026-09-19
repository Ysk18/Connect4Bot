// New file, not part of the graded DSA-project sources.
// Standalone native smoke test for web_bridge.c, compiled with plain gcc
// (no munit dependency) for fast local iteration. See web/README.md for the
// full build command.

#include "web_bridge.h"
#include "connect4.h"

#include <assert.h>
#include <stdio.h>

static void test_fresh_game(void)
{
    wb_start_game(7, 6, 4);
    assert(wb_get_status() == IN_PROGRESS);
    for (int col = 0; col < 7; col++)
        assert(wb_is_move_available(col) == 1);
    printf("PASS: test_fresh_game\n");
}

static void test_horizontal_win(void)
{
    // Player 1 ('X') plays columns 0,1,2,3 on the bottom row; player 2 ('O')
    // plays elsewhere in between. Player 1 wins on the 4th X.
    wb_start_game(7, 6, 4);

    assert(wb_do_human_move(0) == IN_PROGRESS); // X
    assert(wb_do_human_move(0) == IN_PROGRESS); // O (stacks on col 0)
    assert(wb_do_human_move(1) == IN_PROGRESS); // X
    assert(wb_do_human_move(1) == IN_PROGRESS); // O
    assert(wb_do_human_move(2) == IN_PROGRESS); // X
    assert(wb_do_human_move(2) == IN_PROGRESS); // O
    int status = wb_do_human_move(3);           // X completes 0,1,2,3 on bottom row
    assert(status == PLAYER_1_WIN);

    printf("PASS: test_horizontal_win\n");
}

static void test_column_fill_and_draw_guard(void)
{
    wb_start_game(7, 6, 4);
    // Fill column 0 completely (6 moves) with alternating players.
    for (int i = 0; i < 6; i++)
    {
        int status = wb_do_human_move(0);
        assert(status == IN_PROGRESS || status == PLAYER_1_WIN || status == PLAYER_2_WIN);
        if (status != IN_PROGRESS)
            return; // a diagonal/vertical win happened first, which is fine; stop here
    }
    assert(wb_is_move_available(0) == 0);
    for (int col = 1; col < 7; col++)
        assert(wb_is_move_available(col) == 1);

    printf("PASS: test_column_fill_and_draw_guard\n");
}

static void test_bot_move(void)
{
    wb_start_game(7, 6, 4);
    int col = wb_do_bot_move();
    assert(col >= 0 && col < 7);
    assert(wb_get_cell(5, col) == 'X'); // bottom row, dropped piece
    printf("PASS: test_bot_move\n");
}

static void test_node_count_stays_bounded(void)
{
    wb_start_game(7, 6, 4);
    for (int turn = 0; turn < 6; turn++)
    {
        if (wb_get_status() != IN_PROGRESS)
            break;
        wb_do_bot_move();
        if (wb_get_status() != IN_PROGRESS)
            break;
        wb_do_human_move(turn % 7);
        int nodes = wb_get_node_count();
        assert(nodes > 0);
        assert(nodes < 2000000); // generous sanity ceiling at depth 4
    }
    printf("PASS: test_node_count_stays_bounded\n");
}

int main(void)
{
    test_fresh_game();
    test_horizontal_win();
    test_column_fill_and_draw_guard();
    test_bot_move();
    test_node_count_stays_bounded();
    printf("All smoke tests passed.\n");
    return 0;
}
