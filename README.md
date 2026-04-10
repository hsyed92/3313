# SE 3313 Mini Project - Group 6: Resource-Aware OS Design

xv6 (x86) extended with **resource quotas**, **lazy page allocation**, **zero-copy IPC via shared memory**, and an **eco / deep-sleep** scheduler mode. 

**Group:** Freda Zhao, Himanish K Angrish, Theodore Peter Graboski, Syed Humza Islam, Krishan Maheswaran


## Prerequisites (one-time setup)

Development is intended on **Linux or WSL2** (Windows Subsystem for Linux). Native Windows without WSL is not documented here.

Install:

- `build-essential`, `gcc-multilib` (or equivalent **32-bit x86** compile support)
- `qemu-system-x86` (or `qemu-system-i386`)
- `perl` (for `vectors.pl` / boot block signing)

Example (Debian / Ubuntu / WSL):

```bash
sudo apt update
sudo apt install -y build-essential gcc-multilib qemu-system-x86 perl
```

---

## Step-by-step: build and run

1. **Open a terminal** (WSL or Linux).

2. **Navigate to the project folder location, and then go to the xv6 directory:**

   ```bash
   cd xv6-public-master
   ```

3. **Build** kernel, disk images, and user programs:

   ```bash
   make
   ```


4. **Run under QEMU** (graphical window + serial console):

   ```bash
   make qemu
   ```

   **Headless / SSH / no window** (serial only):

   ```bash
   make qemu-nox
   ```

   Exit QEMU: press `Ctrl-a` then `x` (QEMU monitor escape).

5. **At the xv6 shell prompt**, you can run individual feature tests or the integrated demo (see next sections).


## One-command integrated demo

After `make qemu` or `make qemu-nox`, at the xv6 prompt:

```text
demo
```

This runs, in order: `budget_test`, `lazy_test`, `shmtest`, then `suspend 1`, a 30-tick `sleep`, and `suspend 0`.

---

## Running features individually

At the xv6 shell:

```text
lazy_test
budget_test
shmtest
suspend 1
suspend 0
```

For `suspend`, `1` turns eco mode on, `0` turns it off.

---
