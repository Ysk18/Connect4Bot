#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "connect4.h"
#include "interface.h"
#include "tree.h"

// If you want to run the main, you may need to comment out un-implemented functions
// Otherwise you may get "undefined reference" errors.
int main()
{
    /*
    GameState *state = init_game_state(7, 6);
    char c, player_ch;
    bool player = true;
    while (get_game_status(state) == IN_PROGRESS)
    {
        print_available_moves(state);

        player_ch = player ? 'X' : 'O';
        printf("\nEnter your move for %c: ", player_ch);
        c = getchar();
        getchar();
        int move = c - '0';
        GameState *next_state = make_move(state, move);
        player = !player;
        print_game_state(next_state);

        free_game_state(state);
        state = next_state;
    }

    if (get_game_status(state) == DRAW)
    {
        printf("It is a draw");
    }
    else
    {
        printf("Player %d won!\n", get_game_status(state) + 1);
    }

    free_game_state(state);
    */
    play_game(7, 6, 8, true);
    return 0;
}
