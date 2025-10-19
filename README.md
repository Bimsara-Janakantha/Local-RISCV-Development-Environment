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

✅ Verify:
```bash
spike --help
```

---

## 🖥️ 4. Build Proxy Kernel (`pk`)

> Use `--host=riscv64-unknown-elf` — **this is required** for `pk` to work with Spike.

```bash
cd ~
git clone https://github.com/riscv-software-src/riscv-pk
cd riscv-pk
mkdir build
cd build
../configure --prefix=$RISCV --host=riscv64-unknown-elf
make -j$(nproc)
make install
```

✅ Verify `pk` is installed:
```bash
ls $RISCV/riscv64-unknown-elf/bin/pk
```

> 🔸 Note: `pk` is installed to `$RISCV/riscv64-unknown-elf/bin/`, **not** `$RISCV/bin`.

---

## 🧪 5. Test the Setup

Create a test program:

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
$RISCV/bin/riscv64-unknown-elf-gcc -static -o hello hello.c
spike pk hello
```

✅ Expected output:
```
Hello from RISC-V!
```

> 📌 The `-static` flag is **required** because `pk` only supports statically linked newlib binaries.

---

## ▶️ 6. Convenience Script: `run.sh`

Save this as `run.sh` to automate compilation and execution:

```bash
#!/bin/bash
set -e

if [ -z "$RISCV" ]; then
    echo "Error: RISCV not set" >&2; exit 1
fi
if [ $# -ne 1 ]; then
    echo "Usage: $0 <file.c>"; exit 1
fi

C_FILE="$1"
ELF="${C_FILE%.c}.riscv"

"$RISCV/bin/riscv64-unknown-elf-gcc" -static -O2 -o "$ELF" "$C_FILE"
spike pk "$ELF"
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
