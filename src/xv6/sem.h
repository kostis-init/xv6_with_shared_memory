#include "spinlock.h"
struct sem
{
  uint value;
  struct spinlock lk;
  int init; //initialized
};

typedef struct sem sem_t;