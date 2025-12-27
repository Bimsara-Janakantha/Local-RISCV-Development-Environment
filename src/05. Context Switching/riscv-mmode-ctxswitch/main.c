// Implementation of a simple context switching example in RISC-V machine mode.

// Import necessary headers
#include <stdio.h>
#include <stdint.h>
#include "context.h"

// Define stack size and maximum switches
#define STACK_SIZE 4096
#define MAX_SWITCHES 50

// Global task contexts
context_t ctx_task_1;
context_t ctx_task_2;

// Stack for tasks 
static uint8_t task1_stack[STACK_SIZE];
static uint8_t task2_stack[STACK_SIZE];

// Global counters
volatile int counter1 = 0;
volatile int counter2 = 0;

// Function prototypes
void init_context(context_t *ctx, void (*entry)(void), uint8_t *stack_base, size_t stack_size);
static inline uint64_t get_cycles(void);
void do_work(void);
void task1_entry(void);
void task2_entry(void);

// Main function
int main() {
    /* Initialize task context */

    // Context for task 1
    init_context(&ctx_task_1, task1_entry, task1_stack, STACK_SIZE);

    // Context for task 2 
    init_context(&ctx_task_2, task2_entry, task2_stack, STACK_SIZE);

    // Start the first task
    printf("Starting context switching...\n");

    uint64_t start_cycles = get_cycles();

    // Run task1
    task1_entry();

    uint64_t end_cycles = get_cycles();

    printf("Task1 counter: %d\n", counter1);
    printf("Task2 counter: %d\n", counter2);
    printf("Total cycles: %lu\n", end_cycles - start_cycles);
    printf("Avg cycles per switch: %lu\n", (end_cycles - start_cycles) / (2*(MAX_SWITCHES+1))); 

    return 0;
}

// Helper to initialize a context
void init_context(context_t *ctx, void (*entry)(void), uint8_t *stack_base, size_t stack_size) {
    for (int i = 0; i < sizeof(context_t)/sizeof(long); i++)
        ((long*)ctx)[i] = 0;
    ctx->sp = (long)(stack_base + stack_size) & ~15L;
    ctx->ra = (long)entry;
}

// Read cycle CSR
static inline uint64_t get_cycles(void) {
    uint64_t c;
    __asm__ volatile ("rdcycle %0" : "=r" (c));
    return c;
}

// Simulate some work
void do_work(void) {
    for (volatile int i = 0; i < 100; i++);
}

// Task function
void task1_entry(void) {
    while (counter1 < MAX_SWITCHES) {
        counter1++;

        // Simulate work
        do_work();

        // Yield back to main
        printf("Context1 switch %d\n", counter1);
        ctx_switch(&ctx_task_1, &ctx_task_2);
    }
    // Final yield to exit
    ctx_switch(&ctx_task_1, &ctx_task_2);
}

// Task function
void task2_entry(void) {
    while (counter2 < MAX_SWITCHES) {
        counter2++;

        // Simulate work
        do_work();

        // Yield back to main
        printf("Context2 switch %d\n", counter2);
        ctx_switch(&ctx_task_2, &ctx_task_1);
    }
    // Final yield to exit
    ctx_switch(&ctx_task_2, &ctx_task_1);
}



