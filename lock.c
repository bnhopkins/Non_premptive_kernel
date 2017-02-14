/* lock.c: mutual exclusion
 * If SPIN is false, lock_acquire() should block the calling process until its request can be satisfied
 * Your solution must satisfy the FIFO fairness property
 */

#include "common.h"
#include "lock.h"
#include "scheduler.h"

enum {
    SPIN = TRUE,
};

void lock_init(lock_t * l)
{
    // Lock initialized as unlocked
    // Lock will never be unitialized
    l->status = UNLOCKED;

}

/* Helps process acquire a lock and handle if the lock is already held */
void lock_acquire(lock_t * l)
{
    ASSERT(l->status == LOCKED || l->status == UNLOCKED);

    // If lock is locked, block the process, otherwise continue the process
    if (l->status == LOCKED){
        block();
    }
    else{
        l->status = LOCKED;
    }

}

/* Releases the lock when a process calls this funciton and gives the lock to the next
 * process in the blocked queue, and if that is empty, unlocks lock */
void lock_release(lock_t * l)
{
    ASSERT(l->status == LOCKED || l->status == UNLOCKED);

    // If lock is unlocked and there are processes waiting in the blocked queue
    // give the lock to the next process in the queue. Otherwise unlock the
    // lock.
    if (l->status==LOCKED){
        if(blocked_queue !=NULL){
            unblock();
        }
        else{
            l->status = UNLOCKED;
        }
    }
}
