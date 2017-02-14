# os-lab2
Brooke Hopkins, Kenny Song, Martin Slosarik

## PCB Design
The Process Control Block is relatively simple because processes and threads share the same address space. Each PCB has a user stack and registers, and a kernel stack and registers, as well as a task ID and state. It also contains pointers to the next and prev PCBs, if it is placed in a ready or blocked queue. For kernel threads, the user fields are unused. Because this is a non-preemptive kernel, every task will run until it releases control itself by calling the system calls yield() or exit() for processes, or the kernel functions do_yeild() or do_exit() for kernel threads.

## Context Switching
###Context Switching from a Process
To context switch from a process is slightly more complicated because the process is in the user ring and the kernel is in the kernel ring of protection. Although the processes do not run in a less privileged ring in this project, the address spaces in kernel mode are unknown. 

Process calls yield() or exit() in syslib.S, which calls entry point which holds the address of kernel_entry. This overcomes the problem that the process and the kernel theoretically have different permissions (although not in the case of this lab) and more practically the process does not have access to the addresses of functions in the kernel. We switch from user mode into kernel mode in kernel_entry by saving the user stack, registers, and program counter in the PCB and then restoring the kernel stack, registers and program counter from its PCB. 

Then the process enters the kernel and calls the appropriate function (do_yield() or do_exit()). do_yield() or do_exit() is then called in scheduler.c and updates the process state, and then calls scheduler_entry.

scheduler_entry handles the context switch into a new kernel thread, saves the kernel stack registers and program counter into a PCB and then calls the scheduler. The scheduler then puts the current running PCB at the back of the queue if its waiting or discards it if the process completed. 

Then it returns to entry.s where we restore the kernel stack, registers, and program counter from the new current running process. Then return to do_yield() or do_exit(), then return to kernel entry helper and then finally to kernel_entry where the kernel stack is saved and the user stack is restored and returns to new user process.

###Context Switching from a Kernel Thread
The context switching from a thread is similar, except the kernel thread can call do_yield() or do_exit() directly, unlike the process. 

## Scheduler
The main function in scheduler.c is schedule(), which will switch from the current running task to the next task on the ready queue (if the ready queue is empty, we keep running the current task). If the current task terminated and the ready queue is empty, the OS will hang in this function. Switching in this function consists of modifying the current_running pointer, and the ready_queue pointer.

We also have do_yield() and do_exit(), which are very minimal functions that modifies the PCB state and calls scheduler_entry().

We also have block() and unblock(), functions called by lock.c, which halts the current running task and moves it onto the blocked queue, and moves the top blocked task onto the back of the ready queue, respectively.

## Locks
The locks implement mutual exclusion in the style of Birell. It initializes the lock to unlocked. When a process requests the lock (lock_acquire), if the lock is unlocked the process holds the lock and continues with execution. If the lock is locked, the process that requested the lock is placed on the blocked queue and removed from the ready queue to wait until the lock is released. Once a process releases the lock (lock_release), the lock checks if there are any waiting processes in the queue. If so, the lock remains locked and the next process in the queue is moved to the ready queue and holds the lock. If there are no processes in the blocked queue, the lock is unlocked. 

## Timing
The timer measures the time it takes to context switch between threads and between processes. The thread timer measures the time between when the thread calls yield and the next thread starts executing. The process measures the time between the first process call to yield and the second process call.
