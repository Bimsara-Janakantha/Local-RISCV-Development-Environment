#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
    long regs[14];  // s0-s11 (12 regs) + sp + ra
    void (*pc)(void); // simulated program counter (next function to run)
} context_t;

#endif