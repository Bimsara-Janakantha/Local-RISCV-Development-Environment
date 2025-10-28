

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "context.h" 

// ---------- Context definition ----------
#define STACK_SIZE 2048
#define WORK_LIMIT 10
#define REPEATED_WORK 5

// ---------- Global task state ----------
static long stack1[STACK_SIZE / sizeof(long)];
static long stack2[STACK_SIZE / sizeof(long)];

static context_t ctx1, ctx2;
static int current_task = 1;  // 1 -> task1, 2 -> task2
volatile long i = 0, j = 0;
static int task1_done = 0, task2_done = 0;

// ---------- Function declarations ----------
extern void save_context(context_t *ctx);
extern void restore_context(context_t *ctx);

void task1_func(void);
void task2_func(void);
void yield(void);
void task_trampoline(void);

// ---------- Task implementations ----------
void task1_func(void) {
    while (i < WORK_LIMIT*REPEATED_WORK) {
        i++;
        printf("Task 1 \t Work: %ld\n", i);
        if (i % WORK_LIMIT == 0) {
            printf("Task1: %ld\n\n", i);
            yield();
        }
    }
    printf("Task 1 complete!\n\n");
    task1_done = 1;
    yield();
}

void task2_func(void) {
    while (j < WORK_LIMIT*REPEATED_WORK) {
        j++;
        printf("Task 2 \t Work: %ld\n", j);
        if (j % WORK_LIMIT == 0) {
            printf("Task2: %ld\n\n", j);
            yield();
        }
    }
    printf("Task 2 complete!\n\n");
    task2_done = 1;
    yield();
}

// ---------- Yield & context switch ----------
void yield(void) {
    context_t *old, *new;

    if(task1_done && task2_done){
        printf("All tasks completed successfully.\n");
        printf("Simulation finished.\n");
        exit(0);
    }
    else if (current_task == 1) {
        old = &ctx1;
        new = &ctx2;
        current_task = 2;  // Switch to task 2
    } else {
        old = &ctx2;
        new = &ctx1;
        current_task = 1;  // Switch to task 1
    }

    uint64_t start, end;
    asm volatile ("rdcycle %0" : "=r" (start));
    
    printf("oldAddr: 0x%x \t newAddr: 0x%x\n", old, new);
    printf("Context Switching...\n");

    // Context switching
    printf("Saving... \t\t ra=0x%016lu\n", old->regs[13]);
    save_context(old);
    printf("Context Saved!   \t ra=0x%016lu\n", old->regs[13]);

    printf("Restoring... \t\t ra=0x%016lu\n", new->regs[13]);
    restore_context(new);
    printf("Context Restored! \t ra=0x%016lu\n", new->regs[13]);
    
    asm volatile ("rdcycle %0" : "=r" (end));
    printf("Context switch took %lu cycles\n\n", end - start);

    // Goto task_trampoline
    task_trampoline();
}

// ---------- Trampoline to enter task ----------
void task_trampoline(void) {
    if (current_task == 1) {
        task1_func();
    } else {
        task2_func();
    }
    return;
}

// ---------- Initialize context (set up stack & return address) ----------
void init_context(context_t *ctx, long *stack_top) {
    // Initialize all saved registers to 0 (optional)
    for (int i = 0; i < 14; i++) {
        ctx->regs[i] = 0;
    }
}

// ---------- Main ----------
int main() {
    // Set up stacks (point to top)
    long *sp1 = &stack1[STACK_SIZE / sizeof(long) - 10];
    long *sp2 = &stack2[STACK_SIZE / sizeof(long) - 10];

    init_context(&ctx1, sp1);
    init_context(&ctx2, sp2);

    // Start task1
    current_task = 1;
    task_trampoline();

    return 0;
}