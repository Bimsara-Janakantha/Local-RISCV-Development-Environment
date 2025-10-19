# Setup Local RISC-V Development Environment

This guide sets up a **fully local RISC-V toolchain**, **Spike simulator**, and **Proxy Kernel (`pk`)** in your home directory—**without affecting system-wide installations**.

You’ll be able to:
- Compile C programs to RISC-V
- Run them on the Spike ISA simulator using `pk`
- Keep everything self-contained under a single directory

---

## 📁 Prerequisites

Install required Ubuntu packages:

```bash
sudo apt update
sudo apt install -y \
    autoconf automake autotools-dev curl python3 \
    libmpc-dev libmpfr-dev libgmp-dev gawk build-essential \
    bison flex texinfo gperf libtool patchutils bc \
    zlib1g-dev git libexpat-dev
```

---

## 🗂️ 1. Set Installation Directory

All tools will be installed under `~/path/to/project/directory/riscv` (customize as needed):

```bash
cd ~/path/to/project/directory
mkdir -p riscv
cd riscv
```

Add these lines to the end of your shell profile (`~/.bashrc` or `~/.zshrc`):

```bash
export RISCV=$HOME/path/to/project/directory/riscv/riscv-env
export PATH=$RISCV/bin:$PATH
export PATH=$RISCV/riscv64-unknown-linux-gnu/bin:$PATH
```

> This ensures `spike`, `gcc`, and `pk` are available in your terminal.

Source the shell profile

```bash
source ~/.bashrc
```
---

## 🔧 2. Build RISC-V GNU Toolchain

Clone the repository:
```bash
cd ~/path/to/project/directory/riscv
git clone https://github.com/riscv-collab/riscv-gnu-toolchain.git
cd riscv-gnu-toolchain
git submodule update --init --recursive
```

Install the toolchain (Linux):
```bash
./configure --prefix=$RISCV --with-arch=rv64gc --with-abi=lp64d --enable-linux
make linux
make install
```
> This will install the toolchain in the local profile, not system-wide.

Verify:
```bash
$RISCV/bin/riscv64-unknown-linux-gnu-gcc --version
# or
which riscv64-unknown-linux-gnu-gcc
```

---

## 🧪 3. Build Spike (RISC-V ISA Simulator)

Clone the repository:
```bash
cd ~/path/to/project/directory/riscv
git clone https://github.com/riscv-software-src/riscv-isa-sim.git
cd riscv-isa-sim
mkdir build && cd build
```

Install the spike:
```bash
../configure --prefix=$RISCV
make
make install
```
> This will install the spike in the local profile, not system-wide.

Verify:
```bash
spike --help
# or
which spike
```

---

## 🖥️ 4. Build Proxy Kernel (`pk`)

Clone the repository:
```bash
cd ~/path/to/project/directory/riscv
git clone https://github.com/riscv-software-src/riscv-pk.git
cd riscv-pk
mkdir build && cd build
```

Install the proxy kernel:
```bash
../configure --prefix=$RISCV --host=riscv64-unknown-linux-gnu
make
make install
```
> This will install the pk in the local profile, not system-wide.

Verify `pk` is installed:
```bash
ls $RISCV/riscv64-unknown-linux-gnu/bin/pk
# or
which pk
```

> 🔸 Note: `pk` is installed to `$RISCV/riscv64-unknown-linux-gnu/bin/`, **not** `$RISCV/bin`.

---

## 🧾 5. Environment Configuration (.bashrc)

Add the following block to the end of your ~/.bashrc:

```bash
# ============================================================
# RISC-V Toolchain Environment Setup
# ============================================================

# Default: Personal toolchain
export RISCV=$HOME/path/to/project/directory/riscv/riscv-env
export PATH=$RISCV/bin:$PATH
export PATH=$RISCV/riscv64-unknown-linux-gnu/bin:$PATH

# ---------- Helper Aliases ----------

# Temporarily reset PATH to system directories only
alias sysenv='export PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'

# Display current RISC-V environment info
alias riscvinfo='echo -e "Using RISC-V toolchain at: $RISCV\nCompiler: $(which riscv64-unknown-linux-gnu-gcc)\nSpike: $(which spike)\nPK: $(which pk)"'

# ----------- Helper Aliases for auto backup (Optional but good to use) ---------
alias git-backup='~/path/to/script/directory/riscv-backup.sh'
alias git-sync='cd ~/path/to/project/directory/riscv/riscv && git pull origin backup'

# ============================================================
# End of RISC-V section
# ============================================================
```

---

## 🧩 6. Verify Installation

Check which tools are active:
```bash
which riscv64-unknown-linux-gnu-gcc
which spike
which pk

or

riscvinfo
```
> Output should point to either `/opt/riscv/...` or `~/riscv/...` depending on the active toolchain.

---

## 🧪 7. Test the Setup

Create the project directory
```bash
mkdir -p ~/path/to/project/directory/riscv/riscv/src
```
> All the staff are at the `src`.

Create a separate directory for testing 
```bash
mkdir -p ~/path/to/project/directory/riscv/riscv/test
```
> All the tests are here.

Create a test program (`hello.c`):
```c
// hello.c
#include <stdio.h>
int main() {
    printf("Hello from RISC-V!\n");
    return 0;
}
```

Compile and run:
```bash
riscv64-unknown-linux-gnu-gcc -static hello.c -o hello
spike pk hello
```
> If `pk` or `spike` couldn't find the directory, use absolute paths for them.

Expected output:
```
Hello from RISC-V!
```

> 📌 The `-static` flag is **required** because `pk` only supports statically linked binaries.

---

## ▶️ 8. Convenience Script: `run.sh`

Save this as `run.sh` to automate compilation and execution:

```bash
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
```

Make it executable:
```bash
chmod +x run.sh
./run.sh hello.c
```

---

## 🚫 Common Pitfalls

| Issue | Solution |
|------|--------|
| `could not open pk` | Ensure `pk` is built with `--host=riscv64-unknown-elf` and installed to the correct path |
| `spike` not found | Confirm `$RISCV/bin` is in your `PATH` |
| Program crashes or hangs | Always compile with `-static` |
| Using Linux toolchain (`riscv64-linux-gnu-gcc`) | Don’t — `pk` doesn’t support glibc binaries |

---

## 📚 References

- [RISC-V GNU Toolchain](https://github.com/riscv/riscv-gnu-toolchain)
- [Spike ISA Simulator](https://github.com/riscv/riscv-isa-sim)
- [Proxy Kernel (`pk`)](https://github.com/riscv-software-src/riscv-pk)

---

> ✅ All tools are installed **locally**. To uninstall, simply delete `$RISCV` and the source directories.

Happy RISC-V hacking! 💻🔬

--- 

Let me know if you'd like a version that also supports **RV32**, **debugging with GDB**, or **Linux kernel booting with `bbl`**!
