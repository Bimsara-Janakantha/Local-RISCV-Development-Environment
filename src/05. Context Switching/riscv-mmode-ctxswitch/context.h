// context.h
#ifndef CONTEXT_H
#define CONTEXT_H

// Must match order in switch.S
typedef struct {
    long ra;   // x1
    long sp;   // x2
    long s0;   // x8
    long s1;   // x9
    long s2;   // x18
    long s3;   // x19
    long s4;   // x20
    long s5;   // x21
    long s6;   // x22
    long s7;   // x23
    long s8;   // x24
    long s9;   // x25
    long s10;  // x26
    long s11;  // x27
} context_t;

// Assembly functions
void ctx_save(context_t *ctx);
void ctx_switch(context_t *old, context_t *new);

#endif