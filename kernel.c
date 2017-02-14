/* kernel.c */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "th.h"
#include "util.h"

extern void kernel_exit(void);

#include "tasks.c"

pcb_t *current_running;
pcb_t *ready_queue;
pcb_t pcbs[NUM_TASKS];
pcb_t initial_thread;  // Thread that starts the kernel in _start()

/* This function is the entry point for the kernel
 * It must be the first function in the file
 */
void _start(void)
{
    /* Set up the entry point for system calls */
    *ENTRY_POINT = &kernel_entry;

    clear_screen(0, 0, 80, 25);

    // Copy task data from tasks.c into pcbs array
    int i;
    int stack_base = STACK_MIN + STACK_SIZE;  // Stacks start at a high address and grow down
    for (i = 0; i < NUM_TASKS; i++)  {
        // Set PCB metadata
        pcbs[i].pid = i;
        pcbs[i].state = WAITING;

        // Set PCB kernel-mode state (for both processes and kernel threads)
        pcbs[i].kernel_esp = stack_base;
        pcbs[i].kernel_ebp = stack_base;

        // Set PCB user-mode state and instruction pointers
        // Note: processes start from kernel_eip, and then jump to user_eip,
        //       while threads just start from kernel_eip
        if (task[i]->task_type == PROCESS) {
            stack_base += STACK_SIZE;
            pcbs[i].user_esp = stack_base;
            pcbs[i].user_ebp = stack_base;
            pcbs[i].user_eip = task[i]->entry_point;
            pcbs[i].kernel_eip = restore_user_stack;
        } else {
            pcbs[i].kernel_eip = task[i]->entry_point;
        }

        // Link to next and prev PCB as a doubly-linked list
        if (i < NUM_TASKS - 1) {
            pcbs[i].next = &(pcbs[i+1]);
        } else {
            pcbs[i].next = NULL;  // Last PCB
        }
        if (i > 0) {
            pcbs[i].prev = &(pcbs[i-1]);
        } else {
            pcbs[i].prev = NULL;  // First PCB
        }

        stack_base += STACK_SIZE;
    }

    // Check that our stacks are within the memory bounds
    ASSERT(stack_base <= STACK_MAX);

    // Set up doubly-linked list of PCBs as the scheduler queue
    ready_queue = &(pcbs[0]);

    // Set current_running task to head of ready_queue
    initial_thread.state = TERMINATED;  // We never return to the initial thread after entering scheduler_entry()
    current_running = &(initial_thread);

    /* Schedule the first task */
    scheduler_count = 0;
    scheduler_entry();

    /* We shouldn't ever get here */
    ASSERT(0);
}

/* entry.S:kernel_entry calls this function to place syscall number fn from inside the kernel */
void kernel_entry_helper(int fn)
{
    switch (fn) {
        default:
            print_str(1, 1, "Invalid system call: process exiting");
            /* Fall through */
        case SYSCALL_EXIT:
            do_exit();
            break;
        case SYSCALL_YIELD:
            do_yield();
            break;
    }
}

/* Debugging function to prints PCBs of a doubly linked list*/
void print_list(pcb_t *start, int line)
{
    print_str(line, 0, "PRINTING PCB QUEUE");
    line += 1;

    while (start != NULL) {
        // Print PCB info
        print_pcb(start, line);

        // Prepare for next iteration
        start = start->next;
        line += 1;
    }

    print_str(line, 0, "END OF QUEUE");
}

// Prints info about a PCB
void print_pcb(pcb_t *pcb, int line)
{
    // Get PIDs for current, next, and prev PCB
    uint32_t pid = pcb->pid;
    uint32_t prev_pid;
    if (pcb->prev != NULL) { prev_pid = (pcb->prev)->pid; }
    else { prev_pid = -1; }
    uint32_t next_pid;
    if (pcb->next != NULL) { next_pid = (pcb->next)->pid; }
    else { next_pid = -1; }

    // Get state string for current PCB
    char *state;
    if (pcb->state == RUNNING) { state = "RUNNING"; }
    else if (pcb->state == WAITING) { state = "WAITING"; }
    else if (pcb->state == TERMINATED) { state = "TERMINATED"; }
    else if (pcb->state == BLOCKED) { state = "BLOCKED"; }
    else { state = "ERROR"; }

    // Start printing info
    int col = 0;

    if (prev_pid != -1) { print_int(line, col, prev_pid); }
    else { print_str(line, col, "NULL"); }
    col += 5;

    print_str(line, col, "<-");
    col += 3;

    print_char(line, col, '[');
    print_int(line, col+1, pid);
    print_char(line, col+2, ',');
    print_str(line, col+4, state);
    print_char(line, col+4+strlen(state), ']');
    col += 16;

    print_str(line, col, "->");
    col += 3;

    if (next_pid != -1) { print_int(line, col, next_pid); }
    else { print_str(line, col, "NULL"); }
}
