/*
    Title: Measure Trap/Return Overhead - Enhanced Version
    Auther: Janakantha S.M.B.G.
    Last Update: 27 Oct 2025   
    
    Note: This program should return the average cycle count spent for the bare metal program.
*/

#include <stdint.h>

void switch_context(uint64_t *start, uint64_t *end);

// Define dummy task contexts
uint64_t ctx[31] __attribute__((aligned(16))) = {0};

static inline uint64_t rdcycle() {
    uint64_t c;
    asm volatile ("rdcycle %0" : "=r" (c));
    return c;
}

void main() {
    uint64_t start, end;

    // Initialize dummy registers
    ctx[10] = 0x80000100;  // x11 (a0)

    start = rdcycle();
    switch_context(ctx, ctx);  // save to memory and restore back from memory
    end = rdcycle();

    // Note: If we switch to another location, the system will crash because 
    // no program is saved in memory. The memory initially contains garbage values.

    // Output result via memory (no printf in bare-metal)
    // For Spike, we can inspect via debugger or write to known addr
    *(volatile uint64_t*)0x80001000 = end - start;

    // Halt
    while(1);
}