#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
    long regs[14];  // s0-s11 (12 regs) + sp + ra
} context_t;

#endif