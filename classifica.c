#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "classifica.h"
#include <sys/file.h>

char *classifica_file = "championship.txt";

/**
   * @param arg: nickname to search.
   *
   * @return: 1 if the nickname was found, 0 if the nickname was not found,
   * < 0 for an error.
   *
	 * The function searches the nickname in the championship file.
   *
*/
int championship_search(void *arg)
{
    int fd;
    if ((fd = open(classifica_file, O_RDWR | O_CREAT, S_IRWXU)) == -1)
    {
        perror("open file");
        return -1;
    }

    int re;
    int i;
    i = 0;
    char nickname[40];

    while ((re = read(fd, nickname, 40)) > 0)
    {
        if ((strncmp(nickname, arg, strlen(arg))) == 0) //confronto la stringa passata con quella presenta nel file leggendo solo i primi 40 byte
        {
            i = 1; //la stringa  e' presente quindi esco dal ciclo
            break;
        }
        else
        {
            i = 0; //la stringa non e' presente continuo la ricerca
        }
    }

    if(re < 0) {
        perror("read error");
        return re;
    }

    close(fd);
    return i;
}

/**
   * @param nickname: nickname to write.
   *
   * @return: 1 if the nickname was written, -1 for an error
   *
	 * The function writes a nickname in the championship file.
   *
*/
int championship_write(char *nickname)
{
    int fd;
    if ((fd = open(classifica_file, O_RDWR | O_CREAT, S_IRWXU)) == -1)
    {
      perror("open file");
      return -1;
    }

    int readByte = strlen(nickname);
    lseek(fd, 0l, SEEK_END); //mi posizione alla fine del file (anche se il file e' vuoto )

    if (write(fd, nickname, readByte) != readByte)
    {
        perror("errore write");
        return -1;
    }
    else
    {
        //off_t cur_offset = lseek(fd, 0L, SEEK_CUR); //controllo per capire in che posizione mi trovavo prima di fare un salto di 30 byte
        lseek(fd, (30 - strlen(nickname)), SEEK_CUR); //sposto offset di 30

        if (write(fd, "-", 1) != 1)
        {
            perror("errore primo Zero");
            return -1;
        }

        if (write(fd, "0", 1) != 1)
        {
            perror("errore primo Zero");
            return -1;
        }

        lseek(fd, (SEEK_CUR), SEEK_CUR); //sposto l offeset di 1 in modo di creare uno spazio tra gli zero
        if (write(fd, "-", 1) != 1)
        {
            perror("errore primo Zero");
            return -1;
        }

        if (write(fd, "0", 1) != 1)
        {
            perror("errore secondo Zero");
            return -1;
        }

        lseek(fd, (SEEK_CUR), SEEK_CUR);
        if (write(fd, "-", 1) != 1)
        {
            perror("errore primo Zero");
            return -1;
        }

        if (write(fd, "0", 1) != 1)
        {
            perror("errore terzo Zero");
            return -1;
        }

        lseek(fd, (SEEK_CUR), SEEK_CUR);
        if (write(fd, "\n", strlen("\n")) != strlen("\n")) //inserisco la newLine \n
        {
            perror("errore newline zero");
            return -1;
        }

    }

    close(fd);
    return 1;
}

/**
   * @param client_sd: the client socket descriptor.
   *
   * @return: 1 if function succeed, -1 for an error
   *
	 * The function sends the championship file to the client with the sd specified.
   *
*/
int send_championship(int client_sd)
{
    int re;
    char buffer[BUFFSIZE];
    int fd1;

    if ((fd1 = open("championship.txt", O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
      perror("open file");
      return -1;
    }

    while ((re = read(fd1, buffer, BUFFSIZE)) > 0)
    {

        int w = write(client_sd, buffer, re);

				if(w == -1) {
					perror("write");
					return -1;
				}

				if(w == 0) {
					perror("write");
					return -1;
				}

    }

    if(re == -1) {
      perror("read");
      return -1;
    }

    close(fd1);
    return 1;
}

/**
   * @param arg: nickname
   *
   * @return: 1 if victories updated, 0 if the nickname was not found,
   * -1 in case of error
   *
	 * The function updates the number of victories of a player.
   *
*/
int update_victory_to(void *arg)
{
    int fd;
    if ((fd = open(classifica_file, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
      perror("open file");
      return -1;
    }

    int re;
    char nickname[40];
    char punteggio[3];

    while ((re = read(fd, nickname, 40)) > 0) //nickname con 30 spazi in tot (30byte) + spazio (1byte) +0 (1byte) + spazio (1byte) +0 (1byte)+spazio (1byte) +0 (1byte) +\n (1byte) = 30+1+1+1+1+1+1=36byte
    {

        if ((strncmp(nickname, arg, strlen(arg))) == 0) //confronto la stringa passata con quella presenta nel file leggendo solo i byte della stringa passata
        {
            lseek(fd, (SEEK_CUR - 10), SEEK_CUR); //mi posiziono al offset prima del punteggio vittoria
            if ((re = read(fd, punteggio, 2)) > 0)        //questo sposta l offeset di 1 quindi riesco a leggere il punteggio
            {

                int vittoria = atoi(punteggio); //trasformo la stringa in integer
                vittoria = vittoria + 1;
                if (vittoria != 100)
                {
                    char tmp[2] = {0x0};
                    sprintf(tmp, "%d", vittoria);
                    lseek(fd, (SEEK_CUR - 3), SEEK_CUR);            //deve coincidere all lseek sopra, qui mi sposto sempre all offset prima della vittoria
                    if (write(fd, tmp, 2) != 2) //sposta offset sul byte della vittoria
                    {
                        perror("errore Zero");
                        return -1;
                    }
                }
            }
            //la stringa  e' presente quindi esco dal ciclo e ho aggiornato le vittorie
            close(fd);
            return 1;
        }

    }

    if(re < 0) {
      perror("read");
      return -1;
    }

    close(fd);
    return 0;
}

/**
   * @param arg: nickname
   *
   * @return: 1 if defeats updated, 0 if the nickname was not found,
   * -1 in case of error
   *
	 * The function updates the number of defeats of a player.
   *
*/
int update_defeat_to(void *arg)
{
    int fd;
    if ((fd = open(classifica_file, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
      perror("open file");
      return -1;
    }

    int re;
    char nickname[40];
    char punteggio[3];

    while ((re = read(fd, nickname, 40)) > 0)
    {
        if ((strncmp(nickname, arg, strlen(arg))) == 0) //confronto la stringa passata con quella presenta nel file leggendo solo i byte della stringa passata
        {

            lseek(fd, (SEEK_CUR - 7), SEEK_CUR); //mi posiziono al offset prima del puntaggio sconfitta
            if ((re = read(fd, punteggio, 2)) > 0)       //questo sposta l offeset di 1 quindi riesco a leggere il punteggio
            {

                int sconfitta = atoi(punteggio); //trasformo la stringa in integer
                sconfitta = sconfitta + 1;
                if (sconfitta != 100)
                {
                    char tmp[2] = {0x0};
                    sprintf(tmp, "%d", sconfitta);
                    lseek(fd, (SEEK_CUR - 3), SEEK_CUR);            //deve coincidere all lseek sopra, qui mi sposto sempre all offset prima della vittoria
                    if (write(fd, tmp, 2) != 2) //sposta offset sul byte della vittoria
                    {
                        perror("errore Zero");
                        return -1;
                    }
                }
            }
            //la stringa  e' presente quindi esco dal ciclo e ho aggiornato le sconfitte
            close(fd);
            return 1;
        }

    }

    if(re < 0) {
      perror("read");
      return -1;
    }

    close(fd);
    return 0;
}

/**
   * @param arg: nickname
   *
   * @return: 1 if draws updated, 0 if the nickname was not found,
   * -1 in case of error
   *
	 * The function updates the number of draws of a player.
   *
*/
int update_draw_to(void *arg)
{
    int fd;
    if ((fd = open(classifica_file, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
      perror("open file");
      return -1;
    }

    int re;
    char nickname[40];
    char punteggio[3];

    while ((re = read(fd, nickname, 40)) > 0)
    {

        if ((strncmp(nickname, arg, strlen(arg))) == 0) //confronto la stringa passata con quella presenta nel file leggendo solo i byte della stringa passata
        {

            lseek(fd, (SEEK_CUR - 4), SEEK_CUR); //mi posiziono al offset prima del puntaggio pareggi
            if ((re = read(fd, punteggio, 2)) > 0)       //questo sposta l offeset di 1 quindi riesco a leggere il punteggio
            {

                int pareggio = atoi(punteggio); //trasformo la stringa in integer
                pareggio = pareggio + 1;
                if (pareggio != 100)
                {
                    char tmp[2] = {0x0};
                    sprintf(tmp, "%d", pareggio);
                    lseek(fd, (SEEK_CUR - 3), SEEK_CUR);            //deve coincidere all lseek sopra, qui mi sposto sempre all offset prima della vittoria
                    if (write(fd, tmp, 2) != 2) //sposta offset sul byte della vittoria
                    {
                        perror("errore Zero");
                        return -1;
                    }
                }
            }
            //la stringa  e' presente quindi esco dal ciclo e ho aggiornato i pareggi
            close(fd);
            return 1;
        }

    }

    if(re < 0) {
      perror("read");
      return -1;
    }

    close(fd);
    return 0;
}

void *championship_read() //stampa la classifica
{
    int re;
    char buffer[BUFFSIZE];
    int fd1;

    if ((fd1 = open(classifica_file, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)) == -1)
    {
        printf("errore creazione file");
    }

    while ((re = read(fd1, buffer, BUFFSIZE)) > 0) //leggo tutta la riga , il punteggio viene stampato vicino perche' il read salta \0 nel file
    {

        if (write(STDOUT_FILENO, buffer, re) != re)
        {
            perror("write");
        }
    }

    close(fd1);
    return NULL;
}
