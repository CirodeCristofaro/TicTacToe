#include "waitinglist.h"

void init_list(waitinglist * L) {
  L->front = 0;
}

void push(waitinglist * L, player * p) {
  L->waiting_list[L->front] = p;
  L->front++;
}

void pop(waitinglist * L) {
  if(!is_empty(L))
    L->front--;
}

int is_empty(waitinglist * L) {
  int ret = L->front == 0;
  return ret;
}
