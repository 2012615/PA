#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  int OldVal; //the old value of the expr of wp
  char TheExpr[32];

} WP;

void free_wp(int no);

void WPrint();

bool Check();

WP* new_wp(char* args);

#endif

