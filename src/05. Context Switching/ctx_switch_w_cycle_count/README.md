# Simulate Context Switching (C and Assembly)

### 🧠 Key Concepts

In RISC-V (RV64G), the user-level register file has 32 × 64-bit registers:
- x0 = zero (hardwired)
- x1 = return address (ra)
- x2 = stack pointer (sp)
- x8–x9, x18–x27 = callee-saved (s0–s11)
- Others are caller-saved or temporaries.

For a minimal context switch, we must save/restore:
- Callee-saved registers (s0–s11, i.e., x8–x9, x18–x27) — 12 registers
- Stack pointer (sp, x2) — if tasks have separate stacks
- Return address (ra, x1) — if switching from a function call
- Program counter — simulated via a saved label or function pointer

But for simplicity in a cooperative switch, we Save/restore only the callee-saved registers + ra + sp.

---

### 🎯 Goal
- Create two simple functions (`task1`, `task2`) that yield control to each other.
- Implement a **manual context switch** that saves/restores register state.
- Use `rdcycle` to measure how many cycles the switch takes.

---

### 🧱 Step-by-Step Plan

#### 1. Define a Context Structure
We’ll save the **callee-saved registers** (which a real OS would preserve across context switches). On RISC-V RV64G, `s0–s11` (i.e., `x8–x9`, `x18–x27`)

```c
// context.h
#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
    long regs[11];  // s1-s11 (11 regs)
} context_t;

#endif
```

> Note: We use `long` because RISC-V is 64-bit in this toolchain. Here we don't preserve the `s0, ra, sp`. Because after the restoration we plan to jump back to the paused location of the current program, since we need to measure the context. If we change them program will crashed.  

---

#### 2. Write the Context Switch Routine in Assembly

✅ This is a simpler & Practical Approach: Cooperative Yield with Register Save/Restore

`switch.s`

```assembly
.text
.globl save_context
.globl restore_context

# void save_context(context_t *ctx);
save_context:
    sd s1, 1*8(a0)
    sd s2, 2*8(a0)
    sd s3, 3*8(a0)
    sd s4, 4*8(a0)
    sd s5, 5*8(a0)
    sd s6, 6*8(a0)
    sd s7, 7*8(a0)
    sd s8, 8*8(a0)
    sd s9, 9*8(a0)
    sd s10, 10*8(a0)
    sd s11, 11*8(a0)
    ret

# void restore_context(context_t *ctx);
restore_context:
    ld s1, 1*8(a0)
    ld s2, 2*8(a0)
    ld s3, 3*8(a0)
    ld s4, 4*8(a0)
    ld s5, 5*8(a0)
    ld s6, 6*8(a0)
    ld s7, 7*8(a0)
    ld s8, 8*8(a0)
    ld s9, 9*8(a0)
    ld s10, 10*8(a0)
    ld s11, 11*8(a0)
    ret

```
> `ra, sp, s0` are not saved in this section. 

---

#### 3. Write the main program in C

`main.c`

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "context.h" 

// ---------- Context definition ----------
#define WORK_LIMIT 10
#define REPEATED_WORK 5

// ---------- Global task state ----------
static context_t ctx1, ctx2;
static int current_task = 1;  // 1 -> task1, 2 -> task2
static int task1_done = 0, task2_done = 0;

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
    uint64_t start, end;
    asm volatile ("rdcycle %0" : "=r" (start));
    
    save_context(old);
    printf("Current Saved!\n");

    restore_context(new);
    printf("New Restored!\n");
    
    asm volatile ("rdcycle %0" : "=r" (end));
    printf("Context switch took %lu cycles\n\n", end - start);

    task_trampoline();
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
void init_context(context_t *ctx) {
    // Initialize all saved registers to 0 (optional)
    for (int i = 0; i < 11; i++) {
        ctx->regs[i] = 0;
    }
}

// ---------- Main ----------
int main() {
    init_context(&ctx1);
    init_context(&ctx2);

    // Start task1
    current_task = 1;

    // Run context switch
    restore_context(&ctx1);
    task_trampoline();

    return 0;
}

```

> ⚠️ Note: This is a **simplified simulation**. In real OS, you’d use `mret`/`sret` and trap handlers. But under PK, this user-space cooperative switch is valid for **measuring overhead**.

---

### 🧪 Build & Run

```bash
riscv64-unknown-linux-gnu-gcc -static -o ctx_switch main.c switch.s
spike pk ctx_switch
```

You should see:
- Alternating prints from Task1 and Task2.
- One line: `Context switch took XXXX cycles` for each context switching.

> 💡 On Spike, cycle counts are **instruction counts** (not real time), but consistent for relative measurement.

---

### 🔍 What This Measures
- The cost of **saving 11 registers + restoring 11 registers** for complete process.
- **Does NOT include** trap entry/exit (since we’re in user mode).
- For **real OS context switch**, you’d add trap handling cost (which you can measure separately via `ecall` + custom handler in M-mode).
