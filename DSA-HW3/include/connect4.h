// DO NOT MODIFY THIS FILE
// A simple implementation of Connect 4 Game
// Rules:
// - Player 1 is 'X', Player 2 is 'O'
// - Player 1 is always plays first and it is the gamebot, player 2 is either human or random
// - Players take turns placing their token in a column
// - The first player to get 4 in a row/column/diagonal wins

// Note that top left is (0, 0):
// (0, 0) ----> (width-1, 0)
// |
// |
// v
// (0, height-1)

#ifndef CONNECT4_H
#define CONNECT4_H

#include <stdbool.h>

typedef enum
{
    PLAYER_1_WIN,
    PLAYER_2_WIN,
    DRAW,
    IN_PROGRESS
} GameStatus;

typedef struct GameState
{
    char *board;        // 1D array to represent the 2D board: '_' is empty, 'X' is player 1, 'O' is player 2
    bool next_turn;     // Who plays next. false is player 1 ('X'), true is player 2 ('O')
    int evaluation;
    int width;
    int height;
} GameState;

// Initialize a game state with an empty board
GameState *init_game_state(int width, int height);

// Given a board, modifies the "moves" array with the available moves and return the number of available moves
// (You need to create the moves array before calling this function)
int available_moves(GameState *gs, bool *moves);

// Prints the available moves (for human player or debugging)
void print_available_moves(GameState *gs);

// Given a board and a move, allocates a new GameState object with move applied and returns it
GameState *make_move(GameState *gs, int move);

// Given a board, return the game status
GameStatus get_game_status(GameState *gs);

// Frees the game state
void free_game_state(GameState *gs);

// Print a game state
void print_game_state(GameState *gs);

#endif