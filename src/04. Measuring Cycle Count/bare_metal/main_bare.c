/*
    Title: Measure Trap/Return Overhead - Enhanced Version
    Auther: Janakantha S.M.B.G.
    Last Update: 26 Oct 2025   
    
    Note: This program should return the average cycle count spent for the bare metal program.
*/

#include <stdint.h>

void switch_context(uint64_t *save, uint64_t *restore);

// Define dummy task contexts
uint64_t ctx1[31] __attribute__((aligned(16))) = {0};
uint64_t ctx2[31] __attribute__((aligned(16))) = {0};

static inline uint64_t rdcycle() {
    uint64_t c;
    asm volatile ("rdcycle %0" : "=r" (c));
    return c;
}

void main() {
    uint64_t start, end;

    // Initialize dummy registers
    ctx1[10] = 0x1234;  // x11
    ctx2[10] = 0x5678;

    start = rdcycle();
    switch_context(ctx1, ctx2);  // switch to ctx2
    //switch_context(ctx2, ctx1);  // switch back
    end = rdcycle();

    // Output result via memory (no printf in bare-metal)
    // For Spike, we can inspect via debugger or write to known addr
    *(volatile uint64_t*)0x80001000 = end - start;

    // Halt
    while(1);
}