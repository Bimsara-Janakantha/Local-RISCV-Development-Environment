#ifndef CONTEXT_H
#define CONTEXT_H

typedef struct {
    long regs[11];  // s1-s11 (11 regs)
} context_t;

#endif