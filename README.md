# xv6_with_shared_memory
xv6 kernel with the feature of shared memory

Added some syscalls (shmget,shmrem,sem_init,sem_up,sem_down)
new:
shmem.c: -shared memory implementation
sem.c: -semaphores implementation
proc.c: -changed fork, to copy the addresses of under the KERNBASE memory(copyuvm).
-changed exit
vm.c: -changed copyuvm, to copy high addresses
-changed deallocuvm, to not kfree a shared page, if it is being used from another proccess
-added some functions for mappages and unmap_addr(via walkpgdir)
syscall.c:-changed argptr, to accept pointers to high addresses (shared memory)
test.c: -Testing with fork. ($test)
test2.c: -Testing with multiple proccesses. ($test2&;test2&;test2&)

This was an assignment for the class of Operating Systems in DIT of UoA (2017)
