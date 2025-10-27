#include <stdio.h>
#include <stdint.h>
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

// ---------- Function declarations ----------
extern void save_context(context_t *ctx);
extern void restore_context(context_t *ctx);

void task1_func(void);
void task2_func(void);
void yield(void);
void task_trampoline(void);

// ---------- Task implementations ----------
void task1_func(void) {
    volatile long i = 0;
    while (i < WORK_LIMIT*REPEATED_WORK) {
        i++;
        printf("Task 1 \t Work: %ld\n", i);
        if (i % WORK_LIMIT == 0) {
            printf("Task1: %ld\n\n", i);
            yield();
        }
    }
}

void task2_func(void) {
    volatile long j = 0;
    while (j < WORK_LIMIT*REPEATED_WORK) {
        j++;
        printf("Task 2 \t Work: %ld\n", j);
        if (j % WORK_LIMIT == 0) {
            printf("Task2: %ld\n\n", j);
            yield();
        }
    }
}

// ---------- Yield & context switch ----------
void yield(void) {
    context_t *old, *new;

    if (current_task == 1) {
        old = &ctx1;
        new = &ctx2;
        current_task = 2;  // Switch to task 2
    } else {
        old = &ctx2;
        new = &ctx1;
        current_task = 1;  // Switch to task 1
    }
    
    printf("oldAddr: 0x%x \t newAddr: 0x%x\n", old, new);
    printf("Ready to switch\n");

    uint64_t start, end;

    // Count initial cycle
    asm volatile ("rdcycle %0" : "=r" (start));

    // Context switching
    printf("Saving! \t\t sp=0x%016lx \t ra=0x%016lx\n",old->regs[12], old->regs[13]);
    save_context(old);
    printf("Current task saved! \t sp=0x%016lx \t ra=0x%016lx\n",old->regs[12], old->regs[13]);

    //restore_context(old);
    printf("New task loaded!\n");

    // Count final cycle
    asm volatile ("rdcycle %0" : "=r" (end));
    printf("Context switch took %lu cycles\n\n", end - start);
}

// ---------- Trampoline to enter task ----------
void task_trampoline(void) {
    if (current_task == 1) {
        task1_func();
    } else {
        task2_func();
    }
}

// ---------- Initialize context (set up stack & return address) ----------
void init_context(context_t *ctx, long *stack_top) {
    // Initialize all saved registers to 0 (optional)
    for (int i = 0; i < 14; i++) {
        ctx->regs[i] = 0;
    }
    //ctx->regs[12] = (long)stack_top;          // sp
    //ctx->regs[13] = (long)&task_trampoline;   // ra — where to return after restore

    printf("&task_trampoline=0x%016lx\n", ctx->regs[13]);
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
    task1_func();

    return 0;
}