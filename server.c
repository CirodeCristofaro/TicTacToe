#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/ip.h>
#include "tttlib.h"
#include "waitinglist.h"
#include "classifica.h"
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
	player * p1;
	player * p2;
} pthread_arg;

waitinglist L;
pthread_mutex_t wl_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t championship_mutex = PTHREAD_MUTEX_INITIALIZER;

void * manage(void * arg);
void prepare_game(int client_sd, char * nickname);
void * play_game(void * arg);

int main() {

	init_list(&L);
	struct sockaddr_in myaddress, client_addr;
	socklen_t client_len;

	myaddress.sin_family 	    = AF_INET;
	myaddress.sin_port 		    = htons(5001);
	myaddress.sin_addr.s_addr = htonl(INADDR_ANY);

	int sd;
	// Creating a socket
	if( (sd = socket(PF_INET, SOCK_STREAM, 0 )) < 0 ) {
		perror("socket");
		return 1;
	}

	// Assigning the local address to the socket
	if( bind(sd, (struct sockaddr *) &myaddress, sizeof(myaddress)) < 0 ) {
		perror("bind");
		return 1;
	}

	if( listen(sd, 5) != 0) {
		perror("listen");
		return 1;
	}

	// Server loop
	printf("Server running ...\n");

	int client_sd;
	int * thread_sd;
	while(1) {

		client_len = sizeof(client_addr);

		if ( (client_sd = accept(sd, (struct sockaddr *) &client_addr, &client_len)) < 0 ) {
			perror("accept");
			return 1;
		}

		thread_sd = malloc(sizeof(int));
		*thread_sd = client_sd;

		int e;
		pthread_t tid;
		if((e = pthread_create(&tid, NULL, manage, (void *) thread_sd)) != 0) {
			printf("%s", strerror(e));
			return 1;
		}

		if((e = pthread_detach(tid)) != 0) {
			printf("%s", strerror(e));
			return 1;
		}

	}

	return 0;

}

/**
   * @param arg: Client's socket descriptor.
   *
   * @return: void pointer
   *
	 * The function manages a new client connection. Create a new user if
	 * it's a new nickname or makes a login and then manage the client's request.
   *
*/
void * manage(void * arg) {
	int client_sd = (*(int *) arg);

	char nickname[200];
	char cmd;

	// Check if the client already has its nickname in the championship FILE.
	int n = read(client_sd, nickname, 200);
	nickname[n] = '\0';

	// Checking if the nickname exists in championship FILE:
	// - if YES: read player infos from the FILE and write to the client (login done!)
	// - if NOT: create a new player, insert row in the FILE and write to the client (signup done!)

	pthread_mutex_lock(&championship_mutex);

	int is_registred = championship_search(nickname);
	if(is_registred == 1) {

		// Nickname already registred before.
		write(client_sd, "Welcome back", strlen("Welcome back"));

	} else if(is_registred == 0) {

		// New nickname wrote on file
		championship_write(nickname);
		write(client_sd, "New user created", strlen("New user created"));

	} else {

		// ERROR during read or opening file
		printf("%s: Can't connect", nickname);

	}

	pthread_mutex_unlock(&championship_mutex);

	printf("%s: Connected\n", nickname);

	// Check if cmd: [1] 	Play a new game, [2] 	Championship, [0] 	Exit

	// What does the client want to do?
	read(client_sd, &cmd, 1);

	if(cmd == '1') {

		printf("%s: New game request\n", nickname);
		prepare_game(client_sd, nickname);

	} else if(cmd == '2') {

		printf("%s: Championship request\n", nickname);
		pthread_mutex_lock(&championship_mutex);
		send_championship(client_sd);
		pthread_mutex_unlock(&championship_mutex);
		close(client_sd);

	} else {

		printf("%s: Closing connection\n", nickname);
		close(client_sd);

	}

	return NULL;

}

/**
   * @param client_sd: Client's socket descriptor.
   * @param nickname: Corrispondending client's nickname.
   *
   * @return: void
   *
	 * The function prepares a new game request.
	 * Checks if the waiting list is empty or not and then
	 * starts a new game with the players currently connected.
   *
*/
void prepare_game(int client_sd, char * nickname) {

	player * player1;

	// Player init
	player1 = malloc(sizeof(player));
	init_player(player1, nickname, 'N');
	set_sd(player1, client_sd);

	pthread_mutex_lock(&wl_mutex);
	// Waiting list, is it empty?
	if(is_empty(&L)) {

		// "Wait another player
		push(&L, player1);

	} else {

		// Start a new game
		pthread_arg * arg = malloc(sizeof(pthread_arg));
		arg->p1 = player1;
		arg->p2 = L.waiting_list[0];

		int e;
		pthread_t tid;
		if((e = pthread_create(&tid, NULL, play_game, arg)) != 0) {
			printf("%s", strerror(e));
			exit(1);
		}

		pop(&L);	// No one is waiting now;

	}
	pthread_mutex_unlock(&wl_mutex);

}

/**
   * @param arg: pthread_arg type. It contains the player's infos and clints' sd.
   *
   * @return: void pointer
   *
	 * The function manages a game. It updates the championship FILE based on this
	 * game result.
   *
*/
void * play_game(void * arg) {

	pthread_arg * plys = (pthread_arg *) arg;

	// Players init
	player player_O = *(plys->p1);
	set_symbol(&player_O, 'O');

	player player_X = *(plys->p2);
	set_symbol(&player_X, 'X');

	player player_array[2];
	player_array[0] = player_O;
	player_array[1] = player_X;

	write(player_array[0].sd, player_array[1].nickname, strlen(player_array[1].nickname));
	write(player_array[1].sd, player_array[0].nickname, strlen(player_array[0].nickname));

	write(player_array[0].sd, "O", 1);
	write(player_array[1].sd, "X", 1);

	int turn = 0;
	char move = 'N';

	// Init the board
	tris_board board;
	init_board(&board);

	char valid = '0'; // move checker
	int cw;						// winner checker

	do{

		valid = '0';
		turn = !turn;

		// It's turn of player:
		write(player_array[turn].sd, &move, 1);
		write(player_array[turn].sd, "T", 1);

		do {

			// Getting a valid move
			read(player_array[turn].sd, &move, 1);	// opponent's move

			int m = move - '0';

			if( player_move(&(player_array[turn]), &board, m) ) {
				valid = '1';
			}

			// Sending to the client if the move was valid
			write(player_array[turn].sd, &valid, 1);	// valid values '1' if the move was valid, '0' otherwise

		} while(valid == '0');

		printf("Player %s moved at the cell: %c\n", player_array[turn].nickname, move);

		// is there a winner?
		cw = check_win(&board);

		if(cw == -1) {

			// There's a draw
			write(player_array[turn].sd, "D", 1);
			write(player_array[!turn].sd, "D", 1);

			pthread_mutex_lock(&championship_mutex);
			update_draw_to(player_array[turn].nickname);
			update_draw_to(player_array[!turn].nickname);
			pthread_mutex_unlock(&championship_mutex);

		} else if(cw == 1) {

			// There's a winner
			write(player_array[turn].sd, "1", 1);		// Winner
			write(player_array[!turn].sd, "L", 1);	// Loser

			pthread_mutex_lock(&championship_mutex);
			update_victory_to(player_array[turn].nickname);
			update_defeat_to(player_array[!turn].nickname);
			pthread_mutex_unlock(&championship_mutex);

		} else {

			// There is not a winner yet
			write(player_array[turn].sd, "N", 1);

		}

	} while(cw == 0);

	close(player_O.sd);
	close(player_X.sd);
	free(plys->p1);
	free(plys->p2);
	free(arg);
	return NULL;
}
