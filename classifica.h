#ifndef _CLASSIFICA_H
#define _CLASSIFICA_H

#define BUFFSIZE 128

int  championship_search(void *);
int  championship_write();
void *championship_read();
int  update_victory_to(void *);
int  update_defeat_to(void *);
int  update_draw_to(void *);
int  send_championship(int client_sd);

#endif
