/*
    Title: Measure Context Switching Overhead
    Auther: Janakantha S.M.B.G.
    Last Update: 28 Oct 2025   
    
    Note: This program should return the total cycle count spent for the whole task.
          Here, after a context switch all callee-saved registers are saved to the memory (current task) 
          and restore the values from the memory related to the next task. Then executes the newly loaded task.
*/

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
static int task1_done = 0, task2_done = 0;
uint64_t start, end;

// Global Register - That saves the current status of the counter. 
// Here we use the same address for both tasks, then it should be toggle between two tasks.
register long i asm("s1");

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
    while (i < WORK_LIMIT*(REPEATED_WORK+2)) {
        i++;
        printf("Task 2 \t Work: %ld\n", i);
        if (i % WORK_LIMIT == 0) {
            printf("Task2: %ld\n\n", i);
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
        
        asm volatile ("rdcycle %0" : "=r" (end));
        printf("Context switch took %lu cycles\n\n", end - start);
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
    
    printf("oldAddr: 0x%x \t newAddr: 0x%x\n", old, new);
    printf("Context Switching...\n");

    // Context switching
    save_context(old);
    restore_context(new);
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
    long *sp1 = &stack1[STACK_SIZE / sizeof(long) - 1];
    long *sp2 = &stack2[STACK_SIZE / sizeof(long) - 1];

    init_context(&ctx1, sp1);
    init_context(&ctx2, sp2);

    // Start task1
    current_task = 1;

    // Run context switch
    asm volatile ("rdcycle %0" : "=r" (start));
    restore_context(&ctx1);

    return 0;
}