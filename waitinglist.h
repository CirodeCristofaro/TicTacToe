#ifndef _WAITINGLIST_H
#define _WAITINGLIST_H

#include "tttlib.h"

typedef struct {
  player * waiting_list[1];
  int front;
} waitinglist;

void init_list(waitinglist * L);
void push(waitinglist * L, player * p);
void pop(waitinglist * L);
int  is_empty(waitinglist * L);

#endif
