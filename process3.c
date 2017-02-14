#include "common.h"
#include "syslib.h"
#include "util.h"


void _start(void)
{
    static uint64_t curr = 0;
    static uint64_t elapsed_time = 0;
    static uint64_t before = 0;
    static uint32_t first = 0;
    while(1){
        // If it is the first time the proces is called, get the current time and
        // indicate the first process has executed, then yield.
        if (!first){
            before = get_timer();
            first = 1;
            yield();
        }
        // If it is the second time the process is called, get the current time and
        // subtract the previous time to get the elapsed_time. Indicate the second
        // process has executed with first and then yeild
        else{
            curr = get_timer();
            elapsed_time =curr-before;
            print_str(0,40,"Process Context Switch Time: ");
            print_int(0,70,elapsed_time);

            first = 0;
            yield();
        }

    }

}
