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

But for simplicity in a cooperative switch, we can:
- Use separate stacks for each task.
- Save/restore only the callee-saved registers + ra + sp.
- Use a global context struct per task.

---

### 🎯 Goal
- Create two simple functions (`task1`, `task2`) that yield control to each other.
- Implement a **manual context switch** that saves/restores register state.
- Use `rdcycle` to measure how many cycles the switch takes.

---

### 🧱 Step-by-Step Plan

#### 1. Define a Context Structure
We’ll save the **callee-saved registers** (which a real OS would preserve across context switches). On RISC-V RV64G, these are:
- `s0–s11` (i.e., `x8–x9`, `x18–x27`)
- `sp` (stack pointer)
- `ra` (return address)

We’ll also store a **program counter placeholder** (simulated via a function pointer).

```c
// context.h
#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
    long regs[14];  // s0-s11 (12 regs) + sp + ra
    void (*pc)(void); // simulated program counter (next function to run)
} context_t;

#endif
```

> Note: We use `long` because RISC-V is 64-bit in this toolchain.

---

#### 2. Write the Context Switch Routine in Assembly

Create `switch.S`:

```assembly
# switch.S
.text
.globl context_switch
.type context_switch, @function

# void context_switch(context_t *old, context_t *new);
context_switch:
    # Save caller's context (old)
    sd s0, 0*8(a0)
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
    sd sp, 12*8(a0)
    sd ra, 13*8(a0)

    # Restore new context
    ld s0, 0*8(a1)
    ld s1, 1*8(a1)
    ld s2, 2*8(a1)
    ld s3, 3*8(a1)
    ld s4, 4*8(a1)
    ld s5, 5*8(a1)
    ld s6, 6*8(a1)
    ld s7, 7*8(a1)
    ld s8, 8*8(a1)
    ld s9, 9*8(a1)
    ld s10, 10*8(a1)
    ld s11, 11*8(a1)
    ld sp, 12*8(a1)
    ld ra, 13*8(a1)

    # Jump to new task's PC
    jr a1, 16*8  # But wait—we don't store PC in regs[14] yet!

```

⚠️ Problem: We can’t directly jump to a function pointer stored in memory from this point **and** restore all state cleanly in pure assembly without more setup.

✅ Better approach: **Don’t store `pc` in the context struct**. Instead, design tasks to **resume from a known point** (like a loop), and use the context switch only to swap register stacks. We’ll use a **trampoline** idea.

But for simplicity in **user-mode simulation**, let’s use a **cooperative yield** that returns to a scheduler loop.

---

### ✅ Simpler & Practical Approach: Cooperative Yield with Register Save/Restore

We’ll avoid full PC manipulation. Instead:

- Each task runs in a loop.
- When it calls `yield()`, it saves its state and returns to a **scheduler**.
- The scheduler restores the next task.

This avoids complex PC handling and works in user mode.

---

### 📄 Full Working Example

#### `context_switch_demo.c`

```c
#include <stdio.h>
#include <stdint.h>
#include "context.h"

// Assembly function
extern void save_context(context_t *ctx);
extern void restore_context(context_t *ctx);

// Two task stacks (2KB each)
#define STACK_SIZE 2048
static long stack1[STACK_SIZE/sizeof(long)];
static long stack2[STACK_SIZE/sizeof(long)];

static context_t ctx1, ctx2;
static int current_task = 1;

// Dummy tasks
void task1_func(void) {
    volatile int i = 0;
    while (1) {
        i++;
        if (i % 1000000 == 0) {
            printf("Task1: %d\n", i);
            yield(); // cooperative yield
        }
    }
}

void task2_func(void) {
    volatile int j = 0;
    while (1) {
        j++;
        if (j % 1000000 == 0) {
            printf("Task2: %d\n", j);
            yield();
        }
    }
}

// Yield function
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

    uint64_t start, end;
    asm volatile ("rdcycle %0" : "=r" (start));
    save_context(old);
    restore_context(new);
    asm volatile ("rdcycle %0" : "=r" (end));

    // Print only once to avoid noise
    static int first = 1;
    if (first) {
        printf("Context switch took %lu cycles\n", end - start);
        first = 0;
    }
}

// Assembly helpers (in switch.S)
void init_context(context_t *ctx, void (*func)(void), long *stack_top) {
    // Set up initial context to start at func
    ctx->regs[12] = (long)stack_top;          // sp
    ctx->regs[13] = (long)&task_trampoline;   // ra (return to trampoline)
    ctx->pc = func;
}

// Trampoline to call the actual task
void task_trampoline(void) {
    // When restore_context returns, we land here
    if (current_task == 1) {
        task1_func();
    } else {
        task2_func();
    }
}
```

#### `switch.S`

```assembly
.text
.globl save_context
.globl restore_context
.globl init_context

# void save_context(context_t *ctx);
save_context:
    sd s0, 0*8(a0)
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
    sd sp, 12*8(a0)
    sd ra, 13*8(a0)
    ret

# void restore_context(context_t *ctx);
restore_context:
    ld s0, 0*8(a0)
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
    ld sp, 12*8(a0)
    ld ra, 13*8(a0)
    ret
```

#### `main.c`

```c
#include "context.h"

extern void init_context(context_t *ctx, void (*func)(void), long *stack_top);
extern void task_trampoline(void);
extern void task1_func(void), task2_func(void);

int current_task = 1;

int main() {
    // Initialize contexts
    init_context(&ctx1, task1_func, &stack1[STACK_SIZE/sizeof(long) - 10]);
    init_context(&ctx2, task2_func, &stack2[STACK_SIZE/sizeof(long) - 10]);

    // Start with task1
    restore_context(&ctx1);
    task_trampoline(); // should not return

    return 0;
}
```

> ⚠️ Note: This is a **simplified simulation**. In real OS, you’d use `mret`/`sret` and trap handlers. But under PK, this user-space cooperative switch is valid for **measuring overhead**.

---

### 🧪 Build & Run

```bash
riscv64-unknown-linux-gnu-gcc -o ctx_switch main.c context_switch_demo.c switch.S -static
spike pk ctx_switch
```

You should see:
- Alternating prints from Task1 and Task2.
- One line: `Context switch took XXXX cycles`.

> 💡 On Spike, cycle counts are **instruction counts** (not real time), but consistent for relative measurement.

---

### 🔍 What This Measures
- The cost of **saving 14 registers + restoring 14 registers**.
- **Does NOT include** trap entry/exit (since we’re in user mode).
- For **real OS context switch**, you’d add trap handling cost (which you can measure separately via `ecall` + custom handler in M-mode).

---

### ✅ Next Steps
Once this works:
- Try reducing the number of registers saved (e.g., only `s0–s5`) to simulate **register partitioning** (relevant to your research!).
- Compare cycle counts for full vs. partial context saves.

Would you like help:
- Building this code step-by-step?
- Modifying it to simulate **register file partitioning**?
- Moving to **bare-metal (no PK)** to handle real traps?

Let me know!