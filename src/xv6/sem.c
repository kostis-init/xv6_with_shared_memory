#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "sem.h"


void init_sem(struct sem* sem,int value)
{
  initlock(&sem->lk,"sem");
  sem->value = value;
}

void up_sem(struct sem* sem)
{
  acquire(&sem->lk);
  sem->value++;
  wakeup(sem);
  release(&sem->lk);
}

void down_sem(struct sem* sem)
{
  acquire(&sem->lk);
  while(!sem->value)
    sleep(sem, &sem->lk);
  sem->value--;
  release(&sem->lk);
}