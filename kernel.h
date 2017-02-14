/* kernel.h: definitions used by kernel code */

#ifndef KERNEL_H
#define KERNEL_H

#include "common.h"

/* ENTRY_POINT points to a location that holds a pointer to kernel_entry */
#define ENTRY_POINT ((void (**)(int)) 0x0f00)

/* System call numbers */
enum {
    SYSCALL_YIELD,
    SYSCALL_EXIT,
};

/* All stacks should be STACK_SIZE bytes large
 * The first stack should be placed at location STACK_MIN
 * Only memory below STACK_MAX should be used for stacks
 */
enum {
    STACK_MIN = 0x40000,
    STACK_SIZE = 0x1000,
    STACK_MAX = 0x50000,
};

/* The process control block is used for
 * - saving the contents of the registers
 * - maintaining queues of tasks
 * - ...
 */
typedef struct pcb {
    // Task metadata
    uint32_t pid;

    // User-mode task registers

    uint32_t user_edi;
    uint32_t user_esi;
    uint32_t user_ebp;
    uint32_t user_esp;
    uint32_t user_ebx;
    uint32_t user_edx;
    uint32_t user_ecx;
    uint32_t user_eax;
    uint32_t user_eflags;
    uint32_t user_eip;


    // Kernel-mode task registers
    uint32_t kernel_edi;
    uint32_t kernel_esi;
    uint32_t kernel_ebp;
    uint32_t kernel_esp;
    uint32_t kernel_ebx;
    uint32_t kernel_edx;
    uint32_t kernel_ecx;
    uint32_t kernel_eax;
    uint32_t kernel_eflags;
    uint32_t kernel_eip;

    // Pointers to next and prev PCBs in the scheduler queue
    struct pcb *next;
    struct pcb *prev;

    enum { RUNNING, WAITING, TERMINATED, BLOCKED } state;

} pcb_t;

/* Queue of tasks that are ready to run */
extern pcb_t *ready_queue;
/* The task currently running */
extern pcb_t *current_running;

void kernel_entry(int fn);
void kernel_entry_helper(int fn);
void print_list(pcb_t *start, int line);
void print_pcb(pcb_t *start, int line);
void restore_user_stack(void);


#endif                          /* KERNEL_H */
