.text
.globl save_context
.globl restore_context

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

    # sd sp, 12*8(a0) # Do not need to enable this line. 
    # Because after the switching the program doesn't goes to the previous point. 
    # It switch task and continue the remaining process. Thus last stack not neccessary.
    # So it need to reset to the initial point. Otherwise stack overflow can be occured.

    # sd ra, 13*8(a0)  # Do not enable this line. 
    # If this enables the current Return Address saves to the memory. 
    # Then after the next context switching, program will return back 
    # to that location, not to the initial location. Then program will crash.
    
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
    ld sp, 12*8(a0)     # Reset stack pointer
    ld ra, 13*8(a0)     # Reset return address (return to task_trampoline())
    ret
