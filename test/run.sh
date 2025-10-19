#!/bin/bash

# run.sh - Compile and run a C program on RISC-V Spike + PK

set -e  # Exit on any error

# Check RISCV environment variable
if [ -z "$RISCV" ]; then
    echo "Error: RISCV environment variable is not set." >&2
    echo "Please set it to your RISC-V installation path (e.g., export RISCV=\$RISCV)" >&2
    exit 1
fi

# Check arguments
if [ $# -ne 1 ]; then
    echo "Usage: $0 <program.c>" >&2
    exit 1
fi

C_FILE="$1"

# Check if file exists and ends with .c
if [ ! -f "$C_FILE" ]; then
    echo "Error: File '$C_FILE' not found." >&2
    exit 1
fi

if [[ "$C_FILE" != *.c ]]; then
    echo "Warning: Input file does not have a .c extension, but proceeding anyway."
fi

# Derive output name (remove .c extension)
BASENAME="${C_FILE%.*}"
ELF_FILE="${BASENAME}"

# Paths to tools
GCC="$RISCV/bin/riscv64-unknown-linux-gnu-gcc"
SPIKE="$RISCV/bin/spike"
PK="$RISCV/riscv64-unknown-linux-gnu/bin/pk"

# Verify tools exist
for tool in "$GCC" "$SPIKE" "$PK"; do
    if [ ! -x "$tool" ]; then
        echo "Error: Required tool not found or not executable: $tool" >&2
        echo "Make sure you've built and installed the RISC-V toolchain, Spike, and PK correctly." >&2
        exit 1
    fi
done

echo "Compiling $C_FILE → $ELF_FILE ..."
"$GCC" -static "$C_FILE" -o "$ELF_FILE" 

echo "Running on Spike + PK ..."
echo ""

"$SPIKE" "$PK" "$ELF_FILE"

echo ""
echo "Done."