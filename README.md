# xv6_with_shared_memory
###### This was an assignment for the class of Operating Systems in DIT @ University of Athens (2017).
* Modified [xv6 kernel](src/xv6/README) adding **shared memory** & **semaphores**

## Source Code
###### New files have been added and some existing code has been updated.
* [shmem.c](src/xv6/shmem.c): 
  * shared memory implementation
* [sem.c](src/xv6/sem.c): 
  * semaphores implementation
* [proc.c](src/xv6/proc.c): 
  * changed fork to copy the addresses under the KERNBASE memory (copyuvm)
  * changed exit
* [vm.c](src/xv6/vm.c): 
  * changed copyuvm to copy high addresses
  * changed deallocuvm to not kfree a shared page if it is being used from another proccess
  * added some functions for mappages and unmap_addr (via walkpgdir)
* [syscall.c](src/xv6/syscall.c): 
  * changed argptr to accept pointers to high addresses (shared memory)
* [sysproc.c](src/xv6/sysproc.c):
  * added syscalls (shmget, shmrem, sem_init, sem_up, sem_down)

## Build & Run
###### Check the instructions in the original [README](src/xv6/README#L42-L49).
```
$ cd ./src/xv6
$ make
$ make qemu-nox
```

## Test
###### Test xv6 with [test.c](src/xv6/test.c) and [test2.c](src/xv6/test2.c) or create new workloads.
#### Fork
* [test.c](src/xv6/test.c): 
```
// We are inside QEMU running our kernel
$ test
```
#### Multiple processes
* [test2.c](src/xv6/test2.c):
```
// We are inside QEMU running our kernel
$ test2&;test2&;test2&
```

## Exit
###### Terminate QEMU by pressing Ctrl-A X
