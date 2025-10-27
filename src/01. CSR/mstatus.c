/*
    Title: Reading mstatus in C (User Mode)
    Auther: Janakantha S.M.B.G.
    Last Update: 23 Oct 2025    

    Note: This will cause an illegal instruction exception, 
          because user mode cannot access mstatus. 
          Spike will terminate with a trap.
*/

#include <stdio.h>
#include <stdint.h>

static inline uint64_t read_mstatus() {
    uint64_t val;
    asm volatile ("csrr %0, mstatus" : "=r" (val));
    return val;
}

int main(){
    // Check start cpu cycle count
    uint64_t startStts = read_mstatus();
    printf("Start Status: %lu\n", startStts);

    // Do some work
    for (volatile int i=0; i<1000; i++);

    // Check end cpu cycle count
    uint64_t endStts = read_mstatus();

    // Print Results
    printf("End Status: %lu\n", endStts);

    // Terminate Program
    return 0;
}