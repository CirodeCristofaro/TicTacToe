#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "tttlib.h"
#include "classifica.h"

#define MAXSIZE 300
typedef void (*sighandler_t)(int);

int menu();
void play_game(int server_sd, char *nickname);
void show_championship(int server_sd);
void signal_handler(int num_segnale);
int are_args_invalid(int argc, char *argv[], struct sockaddr_in *addr);

int main(int argc, char *argv[]) {

  signal(SIGINT, signal_handler);
  signal(SIGTSTP, signal_handler);

  char nickname[MAXSIZE];
  int sd;
  int input;

  struct sockaddr_in myaddress;
  myaddress.sin_family = AF_INET;

  int invalid_input = are_args_invalid(argc, argv, &myaddress);
  if(invalid_input != 0) {
    return invalid_input;
  }

  printf("Insert your nickname: ");
  scanf("%s", nickname);

  if(strlen(nickname) > 30) {
  	fprintf(stderr, "Please, insert a nickname with 30 characters maximum\n");
  	return 1;
  }

  do {

    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket");
      break;
    }

    if (connect(sd, (struct sockaddr *)&myaddress, sizeof(myaddress)) < 0) {
      perror("connect");
      break;
    }

    char buf[MAXSIZE];
    write(sd, nickname, strlen(nickname));
    buf[read(sd, buf, MAXSIZE)] = '\0';
    printf("server: %s\n", buf);

    input = menu();

      switch (input) {

        case 0:
          write(sd, "0", 1);
          return 0;
          break;

        case 1:
          play_game(sd, nickname);
          break;

        case 2:
          show_championship(sd);
          break;

        default:
          write(sd, "0", 1);
          return 0;
          break;

      }


  } while(1);

  return 0;

}

/**
   * @return: 1 for a game request, 2 to show
   * the championship, 0 to quit the game.
   *
   * The function shows on standard output the
   * main menu and get the user's choise.
   *
*/
int menu() {

  int input;

  do {

    // system("clear");
    printf("\nTic Tac Toe Online\n\n");
    printf("[1] Play Game\n");
    printf("[2] Championship\n");
    printf("[0] Exit\n");
    printf("\nChoose an option: ");

    scanf("%d", &input);

  } while (input < 0 || input > 2);

  return input;
}

/**
   * @param server_sd: Server's socket descriptor.
   *
   * @return: void
   *
   * The function shows the current championship.
   *
*/
void show_championship(int server_sd) {

  char buffer[256];

  // '2' means a championship request for the server
  write(server_sd, "2", 1);
  int re = 0;

  printf("\n-----------------------------\n");
  printf("\nChampionship (Nickname-victories-loses-draws)\n");
  printf("\n-----------------------------\n");

  while((re = read(server_sd, buffer, BUFFSIZE)) > 0) {

    if(re == -1) {
      perror("read");
      exit(1);
    }

    buffer[re] = '\0';
    write(STDOUT_FILENO, buffer, re);

  }

  printf("\n-----------------------------\n");
  close(server_sd);

}

/**
   * @param server_sd: Server's socket descriptor.
   * @param nickname: Nickname currently in session.
   *
   * @return: void
   *
   * The function starts a new game. It sends a new request to the server
   * and then wait another player before starting a new game.
   *
*/
void play_game(int server_sd, char *nickname) {

  // Init the board
  tris_board board;
  init_board(&board);

  // The command '1' is a new game request
  write(server_sd, "1", 1);

  printf("Wait another player.\n\n");

  // When tha game is ready the server answers with your symbol for this game
  // At this point two players are ready to play a game

  char opponent_nickname[30];
  int n = read(server_sd, opponent_nickname, 30);
  opponent_nickname[n] = '\0';
  printf("Your opponent is: %s\n\n", opponent_nickname);

  char symbol;
  read(server_sd, &symbol, 1);
  printf("You're the player with symbol: %c\n", symbol);

  // Init Players
  player player_cur;
  init_player(&player_cur, nickname, symbol);

  char opponent_symbol;
  if (symbol == 'X')
  {
    opponent_symbol = 'O';
  }
  else
  {
    opponent_symbol = 'X';
  }

  char move;                // Player's move
  int m;                    // Player's move as int
  char opponent_move = 'N'; // Opponent Player's move, initially null

  char turn;
  char win = 'N'; // If there're not winners yet its value is 'N'

  do
  {
    // Is it your turn?
    printf("Waiting your turn ...\n");

    // Waiting opponet's move, if it's the first turn it will be an 'N' (null move)
    read(server_sd, &opponent_move, 1);

    // Checking if it's a null move
    if (opponent_move != 'N')
    {
      int om = opponent_move - '0';
      set_cell(&board, om, opponent_symbol);
    }

    if (opponent_move == 'D')
    {
      win = 'D';
      break;
    }

    if (opponent_move == 'L')
    {
      win = 'L';
      break;
    }

    // Waiting server's answer, if it's your turn the server answers with a 'T'
    read(server_sd, &turn, 1);

    // Is it the match finished?
    // If turn it's different then 'T', the game it's finished
    if (turn != 'T')
    {
      break;
    }

    // Printing updated board
    update_board(&board);

    // Getting a valid move from the user
    char valid = '0';
    do
    {

      printf("%s (%c), it's your turn, choose a valid cell (from 1 to 9): ", nickname, symbol);
      scanf(" %c", &move);

      m = move - '0';

      // Sending the move to the server
      write(server_sd, &move, 1);

      // Is it valid?
      read(server_sd, &valid, 1); // if yes, valid == 1

    } while (valid != '1');

    // Has won?
    read(server_sd, &win, 1);

    // Getting the board and printing it
    set_cell(&board, m, player_cur.symbol);
    update_board(&board);

  } while (win == 'N');

  if (win == 'D') {
    printf("You draw!\n");
  } else if (win == '1') {
    printf("You win\n");
  } else {  // L
    printf("You lost\n");
  }

  close(server_sd);
}

/**
   * @param num_segnale: signal
   *
   * @return: void
   *
   * Signals' handler
   *
*/
void signal_handler(int num_segnale)
{
  if (num_segnale == SIGINT)//intercetto CTRL + C
  {
    printf("\nInvalid command, exit to disconnect\n");
  }

  if (num_segnale == SIGTSTP) //intercetto CTRL + Z
  {
    printf("\nInvalid command, exit to disconnect\n");
  }
}

/**
   * @param argc: arguments counter
   * @param argv: arguments values
   * @param addr: address
   *
   * @return: 1 if the number of arguments it's less or greater then 3 (The number of expected arguments is 3),
   *          2 if there's an invald address,
   *          3 if the port is invalid,
   *          0 if all the arguments are valid
   *
   * The function checks if the arguments are valid or not.
   *
*/
int are_args_invalid(int argc, char *argv[], struct sockaddr_in *addr){

    char address[MAXSIZE];
    int port;

    if(argc != 3)
    {
      fprintf(stderr, "Usage: %s <IP> <PORT> \n", argv[0]);
      return 1;
    }
    else
    {

      strcpy(address, argv[1]);

      if (inet_aton(address, &addr->sin_addr) == 0)
      {
        fprintf(stderr, "Invalid address \n ");
        return 2;
      }

      port=atoi(argv[2]);

      if ((port < 5000) || (port > 65535))
      {
        fprintf(stderr, "Invalid port, try one between 5000 and 65535 \n");
        return 3;
      }
      else
      {
        addr->sin_port = htons(port);
      }

    }

	  return 0;
}
