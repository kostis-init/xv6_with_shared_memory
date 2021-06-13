#include "types.h"
#include "user.h"
#include "sem.h"

#define PAGES 3

void printkey(sh_key_t key)
{
  printf(1,"key = |");
  for(int i=0;i<16;i++)
    printf(1,"%d|",key[0]);
  printf(1,"\n");
}


int
main(void)
{
  sem_t* sem[PAGES];
  int*   arr[PAGES];
  sh_key_t key[PAGES];
  
  for (int i=0;i<PAGES;i++)
    for (int j=0;j<16;j++)
      key[i][j]=i;
  
  
    
  for (int i=0;i<PAGES;i++)
  {
    sem[i] = (sem_t*)shmget(key[i]);
    
    //if this was the first proc to call this shmem page, initialize
    int n = nprocs_key(key[i]);
    if (n==1)sem_init(sem[i],1);
    
    if (sem[i]==0) exit(); //error
    
    printf(1,"|proccessPID:%d| just took a shmget, virtual address = %d\n",getpid(),(uint)sem[i]);
    printf(1,"|proccessPID:%d| ",getpid());printkey(key[i]);
    
    arr[i] = (int*)(sem[i]+1);
    
    //sleep(100);
    
    sem_down(sem[i]);
    printf(1,"|proccessPID:%d| sem_down, sem->value=%d\n",getpid(),sem[i]->value);
    
    printf(1,"|proccessPID:%d| initial value: arr[%d][50]=%d\n",getpid(),i,arr[i][50]);
    
    arr[i][50] = 150+i;
    
    printf(1,"|proccessPID:%d| value after assignment: arr[%d][50]=%d\n",getpid(),i,arr[i][50]);
    sem_up(sem[i]);
    printf(1,"|proccessPID:%d| sem_up, sem->value=%d\n",getpid(),sem[i]->value);
    //sleep(100);
    
  }
  for (int i=0;i<PAGES;i++)
    shmrem(key[i]);
  exit();
}
