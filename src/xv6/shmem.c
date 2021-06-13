#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

// shared memory page
struct shmempg {
  sh_key_t key;
  uint addr; //physical address. If 0, the page is free
  struct proc* procs[PROCSHPAGES]; //procs that use this page
  int procaddr[PROCSHPAGES]; //virtual address that the proc keeps this page
};

// shared memory
struct {
  struct spinlock lock;
  struct shmempg pages[NSHPAGES];
} shmem;


//FOR DEBUGGING
void print()
{
  struct shmempg* p;

  //acquire(&shmem.lock);
  cprintf("SHARED MEMORY LAYOUT\n");
  int c=0;
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++,c++)
    if(p->addr)
    {
      cprintf("PAGE NUMBER:%d -> key(repeated 16 times):%d, physical address:%d\n\tPROCCESES THAT USE THIS PAGE:\n",c,(int)p->key[0],p->addr);
      for(int i=0;i<PROCSHPAGES;i++)
	if(p->procs[i])cprintf("\t%d: PID=%d, va=%d\n",i,p->procs[i]->pid,p->procaddr[i]);
      cprintf("\n");
    }
  
  //release(&shmem.lock);
}


void init_shmem(void)
{
  struct shmempg* p;
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)
  {
    p->addr=0;
    memset(p->procs,0,sizeof p->procs);
    memset(p->procaddr,0,sizeof p->procaddr);
    memset(p->key,0,sizeof p->key);
  }
  initlock(&shmem.lock, "shmem");
}

//returns 1 if the keys are the same
int same_keys(sh_key_t a,sh_key_t b)
{
  for(int i=0;i<16;i++)
    if(a[i]!=b[i])//not the same
      return 0;
    
  return 1;
}

//copy a to b
void copy_keys(sh_key_t a,sh_key_t b)
{
  for(int i=0;i<16;i++)
    b[i]=a[i];
}

int get_page(sh_key_t key)
{
  struct shmempg* p;
  int ret_address=0;
  char* mem=0;
  struct proc* proc=myproc();
  
  acquire(&shmem.lock);
  //print();
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)
    if(p->addr && same_keys(p->key,key))
    {
      mem = P2V(p->addr);
      break;
    }
  if(mem == 0)// key not found, create a page
    for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)//find first free page
      if(p->addr == 0)
      {
	mem = kalloc();
	if(mem == 0)
	{
	  cprintf("get_page out of memory\n");
	  release(&shmem.lock);
	  return 0;
	}
	memset(mem, 0, PGSIZE);
	copy_keys(key,p->key);
	p->addr = V2P(mem);
	break;
      }
  if(mem == 0)// no space for new page
  {
    cprintf("ERROR: maximum number of shared pages reached!\n");
    release(&shmem.lock);
    return 0;
  }
  int n;
  for(n=0;n<PROCSHPAGES;n++)// append to the procs of that page
    if(p->procs[n] == 0)
    {
      p->procs[n] = proc;
      break;
    }
  if (n == PROCSHPAGES)// many processes have that page
  {
    cprintf("ERROR: shared page with key: %d, is owned by the maximum number of processes already!\n",key);
    release(&shmem.lock);
    return 0;
  }
  int i;
  for(i=0;i<NSHPAGES;i++)//search for the first empty position in the proc's address space (use bitmap)
  {
    char busy = (proc->sh_pages_map[i/8]  >> (7-i%8)) & 1;
    if(!busy)
    {
      proc->sh_pages_map[i/8] |= 1 << (7-i%8);//change bit to 1
      ret_address = KERNBASE - (i+1)*PGSIZE;//virtual address for the page
      break;
    }
  }
  
  p->procaddr[n]=ret_address;//keep the virtual address that the proc keeps this page
  //map virtual-physical
  if(mappages1(proc->pgdir, (char*)ret_address, PGSIZE, V2P(mem), PTE_W|PTE_U) < 0)
  {
    cprintf("shared memory: mappages error\n");
    release(&shmem.lock);
    return 0;
  }
  //cprintf("EXITING GETPAGE\n");
  //print();
  release(&shmem.lock);
  
  return ret_address;
}

int remove_page(sh_key_t key)
{
  struct shmempg* p;
  struct proc* proc=myproc();
  int i;
  int last=0;
  int va;
  
  
  acquire(&shmem.lock);
  
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)//search pages for key
    if(p->addr && same_keys(p->key,key))//found page
    {
      for(i=0;i<PROCSHPAGES;i++)//search procs of page to remove the proc
	if(p->procs[i] == proc)//found, must delete proc from page
	{
	  p->procs[i]=0;
	  va=p->procaddr[i];//keep va for unmapping
	  p->procaddr[i]=0;
	  break;
	}
      if(i==PROCSHPAGES)//not found this proc, error
      {
	cprintf("ERROR: remove_page, proc not found in page's procs. Probably this key belongs to another proc's page!\n");
	return -1;
      }
      
      //unmap va from proc
      unmap_addr(proc->pgdir,(void*)va);
            

      //change bitmap of proc
      int counter=0;
      for(i = SHMEMSTART; i >= SHMEMEND; i -= PGSIZE,counter++)//search for position of va
	if(i==va)//position found
	{
	  proc->sh_pages_map[counter/8] ^= 1 << (7-counter%8);//change bit to 0
	  break;
	}
      //it was the last proc of this page, must kfree!
      if(num_procs_own_page(p->addr,1) == 0)
      {
	last=1;
	memset(p->key,0,sizeof p->key);
	kfree(P2V(p->addr));
	p->addr=0;
      }
      break;
    }
  //cprintf("EXITING REMOVEPAGE\n");
  //print();
  release(&shmem.lock);
  return !last;
}

//returns the number of procs that have this shared page(by key)
int num_procs_own_page_key(sh_key_t key)
{
  acquire(&shmem.lock);
  int counter=0;
  struct shmempg* p;
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)// search shmem for this page
    if(same_keys(key,p->key))// found
    {
      //cprintf("***********  %d  *********\n",p->key[0]);
      for(int i=0;i<PROCSHPAGES;i++)//walk procs
	if(p->procs[i])//+1 proc uses this page
	  counter++;
      break;
    }
  release(&shmem.lock);
  return counter;
}

//returns the number of procs that have this shared page(by address)
int num_procs_own_page(uint pa,char skip_acquire)
{
  if(skip_acquire == 0)
    acquire(&shmem.lock);
  
  int counter=0;
  struct shmempg* p;
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)// search shmem for this page
    if(p->addr == pa)// found
    {
      for(int i=0;i<PROCSHPAGES;i++)//walk procs
	if(p->procs[i])//+1 proc uses this page
	  counter++;
      break;
    }
  
  if(skip_acquire == 0)
    release(&shmem.lock);
  //cprintf("%d\n",counter);
  return counter;
}

//Update shmem! The pages that cp uses, have to be the same for np
//Returns -1 in failure
//only for fork
int shmem_update_fork(struct proc* np, struct proc* cp)//np = new proc, cp = current proc
{
  //cprintf("update fork proc %s\n",cp->name);
  
  acquire(&shmem.lock);
  int i,j;
  
  struct shmempg* p;
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)// search shmem for pages that proc cp has
    for(i=0;i<PROCSHPAGES;i++)
      if(p->procs[i] == cp)//found, must add np to procs
      {
	
	for(j=0;j<PROCSHPAGES;j++)//search for an empty position
	  if(p->procs[j] == 0)
	  {
	    p->procs[j]=np;
	    p->procaddr[j]=p->procaddr[i];//same virtual address as cp, beacuse of fork (copies exactly the address space)
	    break;
	  }
	if(j==PROCSHPAGES)//not found an empty one, fail
	{
	  release(&shmem.lock);
	  return -1;
	}
      }
  release(&shmem.lock);
  
  return 0;
  
}

//only for exit syscall, to clear the proc's pages
void shmem_remove_proc(void)
{
  struct proc* cp = myproc();
  struct shmempg* p;
  int i;
  
  acquire(&shmem.lock);
  
  for(p = shmem.pages; p < &shmem.pages[NSHPAGES]; p++)// search shmem for pages that proc cp has
  {
    for(i=0;i<PROCSHPAGES;i++)
      if(p->procs[i] == cp)//found, must delete cp from page
      {
	p->procs[i]=0;
	p->procaddr[i]=0;
	break;
      }
    if(i!=PROCSHPAGES)//it had this proccess
      if(num_procs_own_page(p->addr,1) == 0)//clear the page if this proc was the last (kfree will happen later)
      {
	memset(p->key,0,sizeof p->key);
	p->addr=0;
      }
  }
      
  release(&shmem.lock);
  
}