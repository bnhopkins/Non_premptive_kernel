#include "common.h"
#include "scheduler.h"
#include "util.h"
#include "lock.h"
#include "th.h"


volatile int lock = FALSE;
static uint64_t first_thread_time = 0;
static uint64_t second_thread_time = 0;
static uint64_t elapsed_time = 0;
lock_t l;

void thread4(void)
{
    while(1) {
        // Get the time of the first thread and then yeild
        first_thread_time = get_timer();
        do_yield();
    }
}

void thread5(void)
{

    while(1) {
        // Get the time of the second thread and print the difference between the two
        second_thread_time = get_timer();
        elapsed_time = second_thread_time-first_thread_time;

        print_str(0,0,"Thread Context Switch Time: ");
        print_int(0,30,elapsed_time);

        do_yield();

        // Do not record time inbetween thread 5 and thread 4 because other processes run
    }
}
