#include "tttlib.h"
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

/**
   * @param p: player to init
   * @param nickname: player's nickname
   * @param symbol: Symbol in game (O, X) 
   *
   * @return: void
   *
   * The function inits a new player struct.
   *
*/
void init_player(player * p, char nickname[24], char symbol) {

	strcpy(p->nickname, nickname);
	p->symbol = symbol;

}

/**
   * @param p: player to edit
   * @param symbol: Symbol in game (O, X) 
   *
   * @return: void
   *
   * The function sets a new symbol to the player.
   *
*/
void set_symbol(player * p, char symbol) {
  p->symbol = symbol;
}

/**
   * @param p: player to edit
   * @param sd: socket descriptor 
   *
   * @return: void
   *
   * The function sets a socket descriptor to the player.
   *
*/
void set_sd(player * p, int sd) {
  p->sd = sd;
}

/**
   * @param board: Tic Tac Board state
   * @param cell: Board's cell 
   *
   * @return: 1 if the move is valid and the cell is available, 0 if not
   *
   * Checks if the cell is valid and if it's available on the board.
   * A cell is valid if it's 0 < cell < 9
   *
*/
int  is_legal_move(tris_board * board, int cell) {

	if((cell <= 0) || (cell > 9))
		return 0;

	switch(cell) {
        case 1:
        	if(board->cell[0][0] != '-')
        		return 0;
            break;
        case 2:
        	if(board->cell[0][1] != '-')
        		return 0;
            break;
        case 3:
        	if(board->cell[0][2] != '-')
        		return 0;
            break;
        case 4:
        	if(board->cell[1][0] != '-')
        		return 0;
            break;
        case 5:
        	if(board->cell[1][1] != '-')
        		return 0;
            break;
        case 6:
        	if(board->cell[1][2] != '-')
        		return 0;
            break;
        case 7:
        	if(board->cell[2][0] != '-')
        		return 0;
            break;
        case 8:
        	if(board->cell[2][1] != '-')
        		return 0;
            break;
        case 9:
        	if(board->cell[2][2] != '-')
        		return 0;
            break;
	}

	return 1;
}

/**
   * @param p: Player
   * @param board: Tic Tac Board state
   * @param cell: Board's cell 
   *
   * @return: 1 if the move is valid and the cell is available, 0 if not
   *
   * Makes the player's move
   *
*/
int player_move(player * p, tris_board * board, int cell) {

	if(!is_legal_move(board, cell)){
			printf("Not legal\n");
			return 0;
	}

	switch(cell) {
	    case 1:
			board->cell[0][0] = p->symbol;
		    break;
		case 2:
		    board->cell[0][1] = p->symbol;
		    break;
	    case 3:
	        board->cell[0][2] = p->symbol;
	        break;
	    case 4:
	        board->cell[1][0] = p->symbol;
	        break;
	    case 5:
	        board->cell[1][1] = p->symbol;
	        break;
	    case 6:
	        board->cell[1][2] = p->symbol;
	        break;
	    case 7:
	        board->cell[2][0] = p->symbol;
	        break;
	    case 8:
	        board->cell[2][1] = p->symbol;
	        break;
	    case 9:
	        board->cell[2][2] = p->symbol;
	        break;
	}

	return 1;

}

/**
   * @param board: Tic Tac Board state
   * 
   * @return: void
   *
   * Init the Tic Tac Toe board with a no valid value '-'.
   * At the end of that function, all the cells will be value: '-'
   *
*/
void init_board(tris_board * board) {

	char temp = '-';
	for(int i = 0; i < SIZE; i++)
		for(int j = 0; j < SIZE; j++)
			board->cell[i][j] = temp;

}

void set_cell(tris_board * board, int c, char symbol) {

	switch(c) {
	    case 1:
			board->cell[0][0] = symbol;
		    break;
			case 2:
		    board->cell[0][1] = symbol;
		    break;
	    case 3:
	        board->cell[0][2] = symbol;
	        break;
	    case 4:
	        board->cell[1][0] = symbol;
	        break;
	    case 5:
	        board->cell[1][1] = symbol;
	        break;
	    case 6:
	        board->cell[1][2] = symbol;
	        break;
	    case 7:
	        board->cell[2][0] = symbol;
	        break;
	    case 8:
	        board->cell[2][1] = symbol;
	        break;
	    case 9:
	        board->cell[2][2] = symbol;
	        break;
	}

}

/**
   * @param board: Tic Tac Board state
   * 
   * @return: void
   *
   * Prints the updated current situation board
   *
*/
void update_board(tris_board * board) {

	printf("\n");
	printf("     |     |      \n  %c  |  %c  |  %c   \n_____|_____|_____ \n", board->cell[0][0], board->cell[0][1], board->cell[0][2]);
	printf("     |     |      \n  %c  |  %c  |  %c   \n_____|_____|_____ \n", board->cell[1][0], board->cell[1][1], board->cell[1][2]);
	printf("     |     |      \n  %c  |  %c  |  %c   \n     |     |      \n", board->cell[2][0], board->cell[2][1], board->cell[2][2]);

}

/**
   * @param board: Tic Tac Board state
   * 
   * @return: -1 if there is a draw
   *           0 if there're not winners yet
   *           1 if there's a winner
   *
   * Checks if there is a winner or not and if there is a draw.
   *
*/
int check_win(tris_board * board) {

	// Rows check
	if((board->cell[0][0] == board->cell[0][1]) && (board->cell[0][1] == board->cell[0][2])) {
		if(board->cell[0][0] != '-')
			return 1;
	}
	if((board->cell[1][0] == board->cell[1][1]) && (board->cell[1][1] == board->cell[1][2])) {
		if(board->cell[1][0] != '-')
			return 1;
	}
	if((board->cell[2][0] == board->cell[2][1]) && (board->cell[2][1] == board->cell[2][2])) {
		if(board->cell[2][0] != '-')
			return 1;
	}

	// Column check
	if((board->cell[0][0] == board->cell[1][0]) && (board->cell[1][0] == board->cell[2][0])) {
		if(board->cell[0][0] != '-')
			return 1;
	}
	if((board->cell[0][1] == board->cell[1][1]) && (board->cell[1][1] == board->cell[2][1])) {
		if(board->cell[0][1] != '-')
			return 1;
	}
	if((board->cell[0][2] == board->cell[1][2]) && (board->cell[1][2] == board->cell[2][2])) {
		if(board->cell[0][2] != '-')
			return 1;
	}

	// Check diagonals
	if((board->cell[0][0] == board->cell[1][1]) && (board->cell[1][1] == board->cell[2][2])) {
		if(board->cell[0][0] != '-')
			return 1;
	}
	if((board->cell[0][2] == board->cell[1][1]) && (board->cell[1][1] == board->cell[2][0])) {
		if(board->cell[0][2] != '-')
			return 1;
	}

	int draw = 0;
	for(int i = 0; i < SIZE; i++) {
		for(int j = 0; j < SIZE; j++) {
			if(board->cell[i][j] == '-') {
				return 0;
			} else {
				draw = -1;
			}
		}
	}

	return draw;

}
