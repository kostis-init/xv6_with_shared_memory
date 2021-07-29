# xv6_with_shared_memory

* [xv6 kernel](src/xv6/README) improved with the feature of **shared memory** using **semaphores**

###### This was an assignment for the class of Operating Systems in DIT @ University of Athens (2017)

## Source Code

###### New files have been added and some existing code has been updated

* [shmem.c](src/xv6/shmem.c): 
  * shared memory implementation
* [sem.c](src/xv6/sem.c): 
  * semaphores implementation
* [proc.c](src/xv6/proc.c): 
  * changed fork to copy the addresses under the KERNBASE memory (copyuvm) - changed exit
* [vm.c](src/xv6/vm.c): 
  * changed copyuvm to copy high addresses
  * changed deallocuvm to not kfree a shared page if it is being used from another proccess
  * added some functions for mappages and unmap_addr (via walkpgdir)
* [syscall.c](src/xv6/syscall.c): 
  * changed argptr to accept pointers to high addresses (shared memory)
* [sysproc.c](src/xv6/sysproc.c):
  * new syscalls (shmget, shmrem, sem_init, sem_up, sem_down)

## Testing

#### Fork

* [test.c](src/xv6/test.c): 
  * `$test`

#### Multiple processes

* [test2.c](src/xv6/test2.c):
  * `$test2&;test2&;test2&`
