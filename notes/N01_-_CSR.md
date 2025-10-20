Control and Status Registers (CSRs) in RISC-V

1) What is a CSR?
- CSRs are special-purpose registers used to:
    - Control hardware behavior
    - Monitor processor state
    - Configure and access performance counters
- They are not general-purpose registers (not like x1, x2, …).
- Accessed via special instructions: csrr, csrw, csrs, csrc.

2) CSR instructions
- csrr rd, csr     -> Read CSR into rd
- csrw csr, rs     -> Write rs into CSR
- csrs csr, rs     -> Set bits in CSR (logical OR)
- csrc csr, rs     -> Clear bits in CSR (logical AND NOT)

Example:
    csrr t0, cycle        # read cycle counter into t0

3) Important CSRs for performance/timing
- cycle   (XLEN): Counts processor cycles since reset
- time    (XLEN): Counts real-time clock ticks
- instret (XLEN): Counts number of instructions retired
- mcycle  (XLEN): Machine-mode cycle counter (similar to cycle)
- mstatus (XLEN): Machine status (interrupts, privilege, etc.)
- mtvec   (XLEN): Machine trap vector (exception/interrupt handler base)

4) Example: measuring cycles
    csrr t0, cycle        # read cycle count before
    # ... do some code ...
    csrr t1, cycle        # read cycle count after
    sub  t2, t1, t0       # cycles taken

5) Notes
- Some CSRs are read-only (e.g., cycle); others are read/write (e.g., mstatus).
- 32-bit vs 64-bit: On RV32, cycle is 64-bit but accessed using 32-bit regs (use rdcycle/rdcycleh or equivalent).
- Useful for profiling, measuring execution time, and context switching in bare-metal programs.