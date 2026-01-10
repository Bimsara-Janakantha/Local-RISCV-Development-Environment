# **Add a custom RISC-V instruction to the Spike simulator** 

## The new samaple custom instruction (SLL4) shifts a register value by 4 bits (logically left), and want a clear, high-level roadmap before diving into implementation.

Below is a **Step-by-Step Task List** to guide your project from simulation to integration:

---

### 🔹 Phase 0: Understand the RISC-V Instruction Encoding & Spike Structure
1. **Study RISC-V base ISA manual**  
   – Focus on how instructions are encoded (especially I-type, R-type).  
   – Identify an unused/custom opcode/funct3/funct7 combination for your new instruction.
   – Selected Type: R, Opcode: 0110011, func3: 001, func7: 1000000

2. **Explore Spike source code structure**  
   – Key directories/files:  
     - `riscv/` → CPU model, decoding, execution  
     - `riscv/insn_template.cc` / `riscv/insns/` → Per-instruction implementations  
     - `riscv/decode.h` → Instruction decoding logic  
     - `riscv/processor.cc` → Core processor state  
   – Understand how existing shift instructions (`slli`, `srli`, etc.) are implemented.

---

### 🔹 Phase 1: Write a Test Program Using Standard Instructions
3. **Write a C or assembly program** that:
   – Performs a 4-bit shift using existing RISC-V instructions (e.g., `slli x1, x2, 4`).  
   – Compiles with `riscv64-unknown-elf-gcc` (or Linux toolchain if preferred).  
   – Runs correctly on **unmodified Spike**.  
   – Serves as your "golden reference" for behavior.

4. **Verify it works**:  
   ```sh
   riscv64-unknown-elf-gcc -o test_shift test_shift.c
   spike pk test_shift
   ```

---

### 🔹 Phase 2: Define Your Custom Instruction
5. **Choose encoding**:  
   – Example: Use a custom funct3 in the OP-IMM major opcode (like `0x13`) but with a reserved funct3 (e.g., `0x7`).  
   – Or use a custom opcode in the custom instruction space (e.g., `0x2B` for custom-0).  
   – Document: mnemonic (e.g., `s4li rd, rs1`), encoding format, semantics.

---

### 🔹 Phase 3: Modify Spike to Support the New Instruction
6. **Add decoding logic** in `riscv/decode.h` (or relevant decoder file) to recognize your instruction.

7. **Implement execution logic**:  
   – Create a new file in `riscv/insns/`, e.g., `s4li.h`.  
   – Mimic existing shift instructions; perform `rd = rs1 << 4` (or `>>`).

8. **Update build system** (if needed): ensure new insn file is compiled.

9. **Rebuild Spike**:
   ```sh
   ./configure --prefix=/opt/riscv
   make
   make install
   ```

---

### 🔹 Phase 4: Write Assembly Using Your Custom Instruction
10. **Write inline assembly or pure `.S` file** that uses your new instruction (using `.word` or custom assembler support—note: standard `gcc` won’t know your mnemonic yet).

    > ⚠️ Since binutils doesn’t know your instruction, you’ll likely encode it manually with `.word 0x...`.

11. **Assemble and link** using standard toolchain (with `.word` trick).

---

### 🔹 Phase 5: Test & Compare
12. **Run your custom-instruction program on modified Spike**.  
13. **Compare output/state** with the reference program from Phase 1.  
14. **Verify correctness** (register values, side effects, performance if relevant).

---

### 🔹 Optional (Advanced)
15. **Patch Binutils** (gas + objdump) to support your mnemonic.  
16. **Add to RISC-V GNU Toolchain** for seamless compilation.  
17. **Simulate in Verilator or FPGA** if targeting hardware later.

---

### Summary of Key Tasks:
| Step | Task |
|------|------|
| 1 | Study RISC-V encoding & Spike source |
| 2 | Write reference program (standard shifts) |
| 3 | Define custom instruction (mnemonic, encoding) |
| 4 | Modify Spike decoder & add insn implementation |
| 5 | Rebuild Spike |
| 6 | Write test using `.word` to inject custom insn |
| 7 | Run and validate against reference |

Once you confirm this plan aligns with your goal, we can go **task-by-task in detail**, starting with writing the reference program or choosing the instruction encoding—whichever you prefer first.