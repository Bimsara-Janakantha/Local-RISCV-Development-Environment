/*
    Title: Read cycle CSR in C (User Mode)
    Auther: Janakantha S.M.B.G.
    Last Update: 23 Oct 2025    

    Note: This program should be possible to execute
          because the `csrr` is a usermode instruction. 
          That should be execute under pk (proxy kernal).
*/

#include <stdio.h>
#include <stdint.h>

static inline uint64_t rdcycle(){
    uint64_t cycleCount;
    asm volatile ("rdcycle %0" : "=r" (cycleCount));
    return cycleCount;
}

int main(){
    // Check start cpu cycle count
    uint64_t startCount = rdcycle();

    // Do some work
    for (volatile int i=0; i<1000; i++);

    // Check end cpu cycle count
    uint64_t endCount = rdcycle();

    // Cycle count for the work
    uint64_t workCount = endCount - startCount;

    // Print Results
    printf("Start Cycle: %lu\n", startCount);
    printf("End Cycle: %lu\n", endCount);
    printf("Elapsed: %lu\n", workCount);

    // Terminate Program
    return 0;
}