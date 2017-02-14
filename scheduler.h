/* scheduler.h */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "common.h"
#include "kernel.h"

/* Number of times scheduler() was called */
extern int scheduler_count;
extern pcb_t *blocked_queue;

/* Save the context and the kernel stack before calling scheduler
 * This function is implemented in entry.S
 */
void scheduler_entry(void);

/* Change current_running to the next task */
void scheduler(void);

// Returns pointer to last element in PCB queue
pcb_t *get_last(pcb_t *pcb);

/* Schedule another task
 * Call from a kernel thread or kernel_entry_helper()
 */
void do_yield(void);

/* Schedule another task
 * Do not reschedule the current one
 * Call from a kernel thread or kernel_entry_helper()
 */
void do_exit(void);

/* Schedule another task, putting the current one in the specified queue
 * Do not reschedule the current task until it is unblocked
 */
void block(void);

/* Unblock the specified task */
void unblock(void);

#endif                          /* SCHEDULER_H */
