#!/bin/sh
gcc server.c tttlib.c waitinglist.c classifica.c  -pthread -Wall -g -o server
gcc client.c tttlib.c waitinglist.c -pthread -Wall -g -o client
