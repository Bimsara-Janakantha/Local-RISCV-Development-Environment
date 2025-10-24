/*
    Title: Measure Trap/Return Overhead
    Auther: Janakantha S.M.B.G.
    Last Update: 24 Oct 2025   
    
    Note: This program should return the cycle count spent for the getpid.
*/

#include <stdio.h>
#include <stdint.h>

static inline uint64_t rdcycle() {
    uint64_t count;
    asm volatile ("rdcycle %0" : "=r" (count));
    return count;
}

// Make a getpid syscall (non-exiting)
long my_getpid() {
    register long a7 asm("a7") = 172;  // syscall for getpid is 172
    register long a0 asm("a0");
    asm volatile ("ecall" : "=r"(a0) : "r"(a7) : "memory");
    return a0; // Return process id
}

int main() {
    uint64_t start, end, cycles;
    long pid;

    // Warm-up
    my_getpid();

    // Get initial cycle count
    start = rdcycle();

    // Do the work
    pid = my_getpid();

    // Get the final cycle count
    end = rdcycle();

    // Calculate the cycle count for the work
    cycles = end - start;

    // Print results
    printf("PID: %ld\n", pid);
    printf("Trap + return cycles: %lu\n", cycles);

    // End of the program
    return 0;
}