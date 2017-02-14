#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Copied over from scheduler.c, kernel.c, kernel.h, tasks.c for testing

enum {
    KERNEL_ADDR = 0x1000,
    PROC1_ADDR = 0x10000,
    PROC2_ADDR = 0x20000,
    PROC3_ADDR = 0x30000
};

void print_str(int line, int col, char *str)
{
    printf("%s\n", str);
}

void print_char(int line, int col, char c)
{
    printf("%c", c);
}

void print_int(int line, int col, int i)
{
    printf("%d", i);
}

// tasks.c

struct task_info {
    uint32_t entry_point;
    enum {
        KERNEL_THREAD,
        PROCESS,
    } task_type;
};

static struct task_info task4 = { PROC1_ADDR, PROCESS };        /* PROC1_ADDR is defined by the Makefile */
static struct task_info task5 = { PROC2_ADDR, PROCESS };
static struct task_info task6 = { PROC2_ADDR, PROCESS };
static struct task_info *task[] = { &task4, &task5, &task6 };

enum {
    NUM_TASKS = sizeof task / sizeof(struct task_info *),
};

// END: tasks.c

// kernel.c, kernel.h
enum {
    STACK_MIN = 0x40000,
    STACK_SIZE = 0x1000,
    STACK_MAX = 0x50000,
};

typedef struct pcb {
    // Task metadata
    uint32_t pid;
    enum { RUNNING, WAITING, TERMINATED, BLOCKED } state;

    // User-mode task registers
    uint32_t user_eax;
    uint32_t user_ebx;
    uint32_t user_ecx;
    uint32_t user_edx;
    uint32_t user_esi;
    uint32_t user_edi;
    uint32_t user_esp;
    uint32_t user_ebp;
    uint32_t user_eip;
    uint32_t user_eflags;

    // Kernel-mode task registers
    uint32_t kernel_eax;
    uint32_t kernel_ebx;
    uint32_t kernel_ecx;
    uint32_t kernel_edx;
    uint32_t kernel_esi;
    uint32_t kernel_edi;
    uint32_t kernel_esp;
    uint32_t kernel_ebp;
    uint32_t kernel_eip;
    uint32_t kernel_eflags;

    // Pointers to next and prev PCBs in the scheduler queue
    struct pcb *next;
    struct pcb *prev;
} pcb_t;

void print_list(pcb_t *start, int line);

void print_pcb(pcb_t *pcb, int line);

pcb_t *current_running;
pcb_t *ready_queue;
pcb_t initial_thread;
pcb_t pcbs[NUM_TASKS];

void _start(void)
{
    // Copy task data from tasks.c into pcbs array
    int i;
    int stack_base = STACK_MIN + STACK_SIZE;  // Stacks start at a high address and grow down
    for (i = 0; i < NUM_TASKS; i++)  {
        // Set PCB metadata
        pcbs[i].pid = i;
        pcbs[i].state = WAITING;

        // TODO: how to set instruction pointer for threads and processes?

        // Set PCB kernel-mode state (for both processes and kernel threads)
        pcbs[i].kernel_esp = stack_base;
        pcbs[i].kernel_ebp = stack_base;
        // pcbs[i].kernel_eip = task[i]->entry_point; ???

        // Set PCB user-mode state (for processes only)
        if (task[i]->task_type == PROCESS) {
            stack_base += STACK_SIZE;
            pcbs[i].user_esp = stack_base;
            pcbs[i].user_ebp = stack_base;
            // pcbs[i].user_eip = task[i]->entry_point; ???
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
    }

    // Check that our stacks are within the memory bounds
    // ASSERT(stack_base <= STACK_MAX);

    // Set up doubly-linked list of PCBs as a queue
    initial_thread.state = TERMINATED;
    current_running = &initial_thread;

    ready_queue = &(pcbs[0]);

    print_list(ready_queue, 0);
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

    printf("%d <- [%d, %s] -> %d\n", prev_pid, pid, state, next_pid);
}

// END: kernel.c, kernel.h

// scheduler.c

pcb_t *get_last(pcb_t *pcb);

void scheduler(void)
{
    // Check that current task has a valid state
    // ASSERT(current_running->state == WAITING || current_running->state == TERMINATED);

    // Append current_running to the end of the PCB queue
    if (current_running->state == WAITING) {
        if (ready_queue != NULL) {
            pcb_t *ready_queue_tail = get_last(ready_queue);
            ready_queue_tail->next = current_running;
            current_running->prev = ready_queue_tail;
            current_running->next = NULL;
        } else {
            ready_queue = current_running;
            ready_queue->prev = NULL;
            ready_queue->next = NULL;
        }
    }

    // If the PCB queue is empty here, hang the OS
    if (ready_queue == NULL) {
        print_str(1, 1, "No more tasks to run! OS is hanging...");
        while (1) {}
    }

    // Point current_running to next PCB in queue, and adjust ready_queue to the
    // next PCB
    current_running = ready_queue;
    ready_queue = ready_queue->next;
    if (ready_queue != NULL) { ready_queue->prev = NULL; }
    current_running->next = NULL;
    current_running->prev = NULL;
    current_running->state = RUNNING;

    // ++scheduler_count;
}

// Returns pointer to last element in PCB queue
pcb_t *get_last(pcb_t *pcb)
{
    // ASSERT(pcb != NULL);
    while (pcb->next != NULL) {
        pcb = pcb->next;
    }
    return pcb;
}


// END: scheduler.c

int main(void)
{
    _start();

    printf("\nSwitching from initial thread to first task\n");
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from yielded task\n");
    current_running->state = WAITING;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from yielded task\n");
    current_running->state = WAITING;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from yielded task\n");
    current_running->state = WAITING;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from terminated task\n");
    current_running->state = TERMINATED;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from terminated task\n");
    current_running->state = TERMINATED;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from yielded task (empty PCB queue)\n");
    current_running->state = WAITING;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);

    printf("\nSwitching from terminated task\n");
    current_running->state = TERMINATED;
    scheduler();
    printf("current_running: ");
    print_pcb(current_running, 0);
    print_list(ready_queue, 0);
}
