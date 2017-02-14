/* scheduler.c */

#include "common.h"
#include "kernel.h"
#include "scheduler.h"
#include "util.h"

int scheduler_count;
pcb_t *blocked_queue;

// The scheduler() function adds the current task to the PCB queue, and selects
//   the next waiting one as the new current task
// We have two pointers: ready_queue and current_running
//  - ready_queue will point to the head of the PCB queue, i.e. the next task
//    waiting to run
//  - current_running points to the currently running task, taken off the queue
// When context switching, we do the following:
//  1. If current task has yielded, append current_running to the end of the PCB
//     queue, adjust prev and null
//     If current task has terminated, skip this step
//  2. Pop the top PCB off the queue and set it to current_running, set prev and
//     next to null
//  3. Update ready_queue to point the new head of the queue, set prev to NULL
void scheduler(void)
{
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

    ++scheduler_count;
}

// Returns pointer to last element in PCB queue
pcb_t *get_last(pcb_t *pcb)
{
    ASSERT(pcb != NULL);
    while (pcb->next != NULL) {
        pcb = pcb->next;
    }
    return pcb;
}

void do_yield(void)
{
    current_running->state = WAITING;
    scheduler_entry();
}

void do_exit(void)
{
    current_running->state = TERMINATED;
    scheduler_entry();
}

// Add current_running onto blocked_queue and switch to the next task
void block()
{
    // Append current_running to blocked_queue
    if (blocked_queue != NULL) {
        pcb_t *blocked_queue_tail = get_last(blocked_queue);
        blocked_queue_tail->next = current_running;
        current_running->prev = blocked_queue_tail;
    } else {
        blocked_queue = current_running;
    }

    // Switch to next task
    current_running->state = BLOCKED;
    scheduler_entry();
}

// Move top task on blocked_queue onto back of ready_queue
void unblock()
{
    // Take task off blocked_queue
    pcb_t *unblocked_task = blocked_queue;
    blocked_queue = blocked_queue->next;
    if (blocked_queue != NULL) { blocked_queue->prev = NULL; }

    // Append task to ready_queue
    if (ready_queue != NULL) {
        pcb_t *ready_queue_tail = get_last(ready_queue);
        ready_queue_tail->next = unblocked_task;
        unblocked_task->prev = ready_queue_tail;
        unblocked_task->next = NULL;
        unblocked_task->state = WAITING;
    } else {
        ready_queue = unblocked_task;
        ready_queue->next = NULL;
        ready_queue->prev = NULL;
    }
}
