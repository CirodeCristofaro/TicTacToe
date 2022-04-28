#ifndef _TTTLIB_H
#define _TTTLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define SIZE 3

typedef struct {
	char cell[SIZE][SIZE];
} tris_board;

typedef struct {
	char nickname[24];
	char symbol;
  	int  sd;
} player;

void init_player(player * p, char nickname[24], char symbol);	
void set_symbol(player * p, char symbol);
void set_sd(player * p, int sd);
int  player_move(player * p, tris_board * board, int cell);
int  is_legal_move(tris_board * board, int cell);

void init_board(tris_board * board);
void update_board(tris_board * board);
void set_cell(tris_board * board, int c, char symbol);
int  check_win(tris_board * board);

#endif
