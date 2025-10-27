#include <stdio.h>
#include <stdint.h>

// ---------- Context definition ----------
#define STACK_SIZE 2048

typedef struct {
    long regs[14];  // s0-s11 (12), sp (1), ra (1)
} context_t;

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
    while (1) {
        i++;
        if (i % 1000000 == 0) {
            printf("Task1: %ld\n", i);
            yield();
        }
    }
}

void task2_func(void) {
    volatile long j = 0;
    while (1) {
        j++;
        if (j % 1000000 == 0) {
            printf("Task2: %ld\n", j);
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
        current_task = 2;
    } else {
        old = &ctx2;
        new = &ctx1;
        current_task = 1;
    }

    static int measured = 0;
    uint64_t start, end;
    if (!measured) {
        asm volatile ("rdcycle %0" : "=r" (start));
    }

    save_context(old);
    restore_context(new);

    if (!measured) {
        asm volatile ("rdcycle %0" : "=r" (end));
        printf("Context switch took %lu cycles\n", end - start);
        measured = 1;
    }
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
    ctx->regs[12] = (long)stack_top;          // sp
    ctx->regs[13] = (long)&task_trampoline;   // ra — where to return after restore
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
    restore_context(&ctx1);
    task_trampoline(); // should not return

    return 0;
}