// main.c
#include <stdio.h>
#include <stdint.h>
#include "context.h"

#define STACK_SIZE 4096
#define MAX_SWITCHES 50

// Global task contexts
context_t ctx_main;
context_t ctx_task2;

// Stack for task2
static uint8_t task2_stack[STACK_SIZE];

volatile int counter1 = 0;
volatile int counter2 = 0;

// Task function
void task2_entry(void) {
    while (counter2 < MAX_SWITCHES) {
        counter2++;
        // Simulate work
        for (volatile int i = 0; i < 100; i++);
        // Yield back to main

        printf("Context2 switch %d\n", counter2);
        ctx_switch(&ctx_task2, &ctx_main);
    }
    // Final yield to exit
    ctx_switch(&ctx_task2, &ctx_main);
}

// Read cycle CSR
static inline uint64_t get_cycles(void) {
    uint64_t c;
    __asm__ volatile ("rdcycle %0" : "=r" (c));
    return c;
}

int main() {
    // Initialize task2 context  
    // Zero other registers (good practice)
    for (int i = 0; i < sizeof(context_t) / sizeof(long); i++) {
        ((long*)&ctx_task2)[i] = 0;
    }
    
    // Stack grows downward; align to 16-byte boundary
    ctx_task2.sp = (long)(task2_stack + STACK_SIZE) & ~15L;
    ctx_task2.ra = (long)task2_entry;

    printf("Starting context switching...\n");

    uint64_t start_cycles = get_cycles();

    // Run task1 (main loop)
    while (counter1 < MAX_SWITCHES) {
        counter1++;
        for (volatile int i = 0; i < 100; i++);

        // Switch to task2
        printf("Context1 switch %d\n", counter1);
        ctx_switch(&ctx_main, &ctx_task2);
    }

    uint64_t end_cycles = get_cycles();

    printf("Task1 counter: %d\n", counter1);
    printf("Task2 counter: %d\n", counter2);
    printf("Total cycles: %lu\n", end_cycles - start_cycles);
    printf("Avg cycles per switch: %lu\n", (end_cycles - start_cycles) / 2000); // ~2000 switches

    return 0;
}