# Explanation of `src/riscv64/hypervisor.ld`

This document explains the linker script for the NOVA microhypervisor on RISC-V 64-bit. This script acts as the blueprint for how the compiler and linker organize the kernel's code and data in memory.

Since NOVA is a freestanding kernel (it runs directly on hardware without an OS), it must strictly define where every byte sits in physical memory (to be loaded) and virtual memory (where it executes).

## 1. Physical vs. Virtual Memory
The script manages two types of addresses using the `AT(...)` keyword:
*   **VMA (Virtual Memory Address):** Where the code *expects* to be running (e.g., `LINK_ADDR` + `OFFSET`).
*   **LMA (Load Memory Address):** Where the binary is physically loaded in RAM (e.g., `LOAD_ADDR`).

The pattern `SECTION . + OFFSET : AT (ADDR (SECTION) - OFFSET)` tells the linker: "Put this section at a high virtual address (`+ OFFSET`), but store it in the physical binary at a lower address (`- OFFSET`)." This allows the kernel to enable paging and "jump" to high memory early in the boot process.

## 2. Section Breakdown

| Section | Purpose |
| :--- | :--- |
| **`.init`** | The very first code to run (the "boot strap"). It runs with 1:1 memory mapping before paging is fully active. |
| **`.text`** | The actual executable code of the hypervisor. |
| **`.rodata`** | **R**ead-**O**nly **data** (constants, strings). |
| **`.init_array`** | **Crucial for C++.** This contains pointers to global constructors (the `CTORS` symbols). Since there is no OS to call them, NOVA's startup code iterates through this list to initialize global C++ objects. |
| **`.data`** | Initialized global variables (e.g., `int x = 5;`). |
| **`.bss`** | Uninitialized global variables (e.g., `int y;`). Does not take up space in the file on disk, but reserves RAM at runtime. |

## 3. Key Kernel Structures (in `.bss`)
The script reserves space for specific low-level hardware structures inside the `.bss` section:
*   `STACK`: The initial boot stack for the first CPU.
*   `PTAB_HPAS`: **P**age **Tab**le **H**ypervisor **P**hysical **A**ddress **S**pace. The root page table for the kernel.
*   `KMEM_HVAS`: The start of the kernel's dynamic memory heap.

## 4. Per-CPU Optimization (`.cpulocal`)
The script includes a specialized section for Per-CPU data:

```ld
.cpulocal MMAP_CPU_DATA :
{
    *(SORT_BY_ALIGNMENT (.cpulocal.hot))
    *(SORT_BY_ALIGNMENT (.cpulocal))
}
```

*   **Purpose:** In a multi-core kernel, valid mostly lock-free access to CPU-specific data (like `current_thread`) is required. NOVA places these variables here. At runtime, a copy of this section is allocated for *each* CPU core.
*   **`.cpulocal.hot`:** A designated subsection for data accessed **extremely frequently** (e.g., on every interrupt or context switch). Grouping "hot" data at the start of the section maximizes **cache locality** (keeping them in the same L1 cache line).

Finally, this assertion ensures the per-CPU data fits within a single page:
```ld
. = ASSERT (SIZEOF (.cpulocal) <= PAGE_SIZE (0), "CPU-Local section too large");
```
