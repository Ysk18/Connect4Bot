// DO NOT MODIFY THIS FILE
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "connect4.h"

GameState *init_game_state(int width, int height)
{
    GameState *state = (GameState *)malloc(sizeof(GameState));

    state->board = (char *)malloc(height * width * sizeof(char));
    for (int i = 0; i < height * width; i++)
    {
        state->board[i] = '_';
    }

    state->next_turn = false;
    state->evaluation = 0;
    state->width = width;
    state->height = height;
    return state;
}

int available_moves(GameState *gs, bool *moves)
{
    int count = 0;
    for (int i = 0; i < gs->width; i++)
    {
        if (gs->board[0 * gs->width + i] == '_')
        {
            moves[i] = true;
            count++;
        }
    }
    return count;
}

void print_available_moves(GameState *gs)
{
    bool moves[gs->width];
    memset(moves, 0, gs->width * sizeof(bool));
    available_moves(gs, moves);
    printf("Available moves: ");
    for (int i = 0; i < gs->width; i++)
    {
        if (moves[i])
        {
            printf("%d ", i);
        }
    }
}

GameState *make_move(GameState *gs, int move)
{
    // Validate move
    if (move < 0 || move >= gs->width)
    {
        printf("[ERROR] Invalid move\n");
        return NULL;
    }

    // Check if column is full
    if (gs->board[0 * gs->width + move] != '_')
    {
        printf("[ERROR] Column is full\n");
        return NULL;
    }

    GameState *next_gs = (GameState *)malloc(sizeof(GameState));
    memcpy(next_gs, gs, sizeof(GameState));
    next_gs->next_turn = !next_gs->next_turn;

    next_gs->board = (char *)malloc(gs->height * gs->width * sizeof(char));
    memcpy(next_gs->board, gs->board, gs->height * gs->width * sizeof(char));

    // Place piece in lowest empty position
    for (int i = gs->height - 1; i >= 0; i--)
    {
        if (gs->board[i * gs->width + move] == '_')
        {
            next_gs->board[i * gs->width + move] = gs->next_turn ? 'O' : 'X';
            break;
        }
    }
    return next_gs;
}

GameStatus get_game_status(GameState *gs)
{
    // Check horizontal wins
    if (gs->width >= 4)
    {
        for (int i = 0; i < gs->height; i++)
        {
            for (int j = 0; j <= gs->width - 4; j++)
            {
                int index = i * gs->width + j;
                if (gs->board[index] != '_' &&
                    gs->board[index] == gs->board[index + 1] &&
                    gs->board[index] == gs->board[index + 2] &&
                    gs->board[index] == gs->board[index + 3])
                {
                    return gs->board[index] == 'X' ? PLAYER_1_WIN : PLAYER_2_WIN;
                }
            }
        }
    }

    // Check vertical wins
    if (gs->height >= 4)
    {
        for (int i = 0; i <= gs->height - 4; i++)
        {
            for (int j = 0; j < gs->width; j++)
            {
                int index = i * gs->width + j;
                if (gs->board[index] != '_' &&
                    gs->board[index] == gs->board[index + gs->width] &&
                    gs->board[index] == gs->board[index + 2 * gs->width] &&
                    gs->board[index] == gs->board[index + 3 * gs->width])
                {
                    return gs->board[index] == 'X' ? PLAYER_1_WIN : PLAYER_2_WIN;
                }
            }
        }
    }

    // Check diagonal wins (top-left to bottom-right)
    if (gs->height >= 4 && gs->width >= 4)
    {
        for (int i = 0; i <= gs->height - 4; i++)
        {
            for (int j = 0; j <= gs->width - 4; j++)
            {
                int index = i * gs->width + j;
                if (gs->board[index] != '_' &&
                    gs->board[index] == gs->board[index + gs->width + 1] &&
                    gs->board[index] == gs->board[index + 2 * gs->width + 2] &&
                    gs->board[index] == gs->board[index + 3 * gs->width + 3])
                {
                    return gs->board[index] == 'X' ? PLAYER_1_WIN : PLAYER_2_WIN;
                }
            }
        }
    }

    // Check diagonal wins (top-right to bottom-left)
    if (gs->height >= 4 && gs->width >= 4)
    {
        for (int i = 0; i <= gs->height - 4; i++)
        {
            for (int j = gs->width - 1; j >= 3; j--)
            {
                int index = i * gs->width + j;
                if (gs->board[index] != '_' &&
                    gs->board[index] == gs->board[index + gs->width - 1] &&
                    gs->board[index] == gs->board[index + 2 * gs->width - 2] &&
                    gs->board[index] == gs->board[index + 3 * gs->width - 3])
                {
                    return gs->board[index] == 'X' ? PLAYER_1_WIN : PLAYER_2_WIN;
                }
            }
        }
    }

    // Check for draw (board is full)
    bool is_full = true;
    for (int j = 0; j < gs->width; j++)
    {
        if (gs->board[0 * gs->width + j] == '_')
        {
            is_full = false;
            break;
        }
    }
    if (is_full)
    {
        return DRAW;
    }

    // Game is still ongoing
    return IN_PROGRESS;
}

void print_game_state(GameState *gs)
{
    for (int i = 0; i < gs->height; i++)
    {
        printf("| ");
        for (int j = 0; j < gs->width; j++)
        {
            printf("%c ", gs->board[i * gs->width + j]);
        }
        printf("|\n");
    }
    printf("\n");
}

void free_game_state(GameState *gs)
{
    free(gs->board);
    gs->board = NULL;
    free(gs);
    gs = NULL;
}