# RISC-V 64-bit Hypervisor Linker Script — Detailed Explanation

Reference: `src/riscv64/hypervisor.ld`

---

## Preamble (lines 19–24)

```
#include "arch.hpp"      →  BFD_ARCH  = "riscv",  BFD_FORMAT = "elf64-littleriscv"
#include "memory.hpp"    →  LOAD_ADDR, LINK_ADDR, OFFSET, PAGE_SIZE(), MMAP_CPU_*
```

The linker script is **preprocessed as C** so it can reuse the same address constants the kernel code uses. `OUTPUT_ARCH` / `OUTPUT_FORMAT` / `ENTRY` set the ELF metadata and the entry point symbol (`__init_bsp` — the BSP boot path).

---

## Program Headers (lines 27–30)

```
init  PT_LOAD FLAGS(5)   → R+X  (readable + executable)
kern  PT_LOAD FLAGS(6)   → R+W  (readable + writable)
```

Two ELF LOAD segments: one for early init code (execute-only), one for the main kernel (read/write for data). The bootloader / SBI uses these to place the image in RAM.

---

## `.init` section (lines 36–44)

```
. = LOAD_ADDR;
PROVIDE_HIDDEN (NOVA_HPAS = LINK_ADDR);
```

- The **location counter** starts at `LOAD_ADDR` (physical: `RAM_BASE + 0x200000`, or `0` if no board config).
- **`NOVA_HPAS`** ("NOVA Hypervisor Physical Address Start") is set to `LINK_ADDR` (`0xffffffffc0000000`). It's expressed as a virtual address here; C++ code uses `Kmem::sym_to_phys()` to convert it back to physical. It marks the **beginning of the hypervisor image**.
- `.init` collects all early-boot code (`.init` and `.init.*` input sections), assigned to the `init` (R+X) segment.

---

## `.text` section (lines 46–50)

```
.text . + OFFSET : AT (ADDR (.text) - OFFSET)
```

This is the key **virtual ↔ physical split**:

| | Value |
|---|---|
| `LINK_ADDR` | `0xffffffffc0000000` (virtual) |
| `LOAD_ADDR` | physical load address |
| `OFFSET` | `LINK_ADDR - LOAD_ADDR` |

- The VMA (virtual memory address) jumps by `OFFSET`, so `.text` runs at high virtual addresses.
- `AT(...)` keeps the LMA (load memory address) physical, so the bootloader loads it contiguously after `.init`.
- Hot text comes first (`text.hot`), then the rest. Assigned to `kern` segment.

---

## `.rodata` / `.init_array` / `.data` (lines 52–74)

All follow the same `. : AT (ADDR(...) - OFFSET)` pattern — virtual addresses for execution, physical for loading.

**`.init_array`** deserves special attention: it collects **static constructors** sorted by priority, exposing boundary symbols:

| Symbol | Purpose |
|---|---|
| `CTORS_L` | Start of "late" constructors (priority 65534) |
| `CTORS_C` | "Core" constructors (priority 65533) |
| `CTORS_S` | All remaining sorted constructors |
| `CTORS_E` | End of constructors |

The kernel walks these ranges to call constructors in the correct initialization order.

---

## `.bss` section (lines 76–94)

The zeroed-data section. It carves out several important regions:

| Symbol | Purpose |
|---|---|
| `__bss_start` | Start of BSS (zeroed at boot) |
| `STACK` | Kernel bootstrap stack (4 KiB, aligned) — note the `- OFFSET` gives a physical address |
| `PTAB_HPAS` | Physical address of the initial page tables |
| `PT2H_HPAS` | Level-2 hypervisor page table (4 KiB) |
| `PT1H_HPAS` | Level-1 hypervisor page table (4 KiB) |
| `KMEM_HVAS` | Start of the kernel memory (buddy allocator) virtual region |
| `__bss_end` | End of BSS, aligned to a level-1 page (2 MiB megapage) with 8 MiB padding |

The `. = ALIGN(ABSOLUTE(.) + 8M, PAGE_SIZE(1))` reserves 8 MiB for early kernel heap / buddy pages and aligns the end to a megapage boundary.

---

## `NOVA_HPAE` (line 96)

```
PROVIDE_HIDDEN (NOVA_HPAE = . - OFFSET);
```

"NOVA Hypervisor Physical Address End" — the physical end of the entire hypervisor image. Together with `NOVA_HPAS`, this defines the physical memory footprint the hypervisor occupies.

---

## Per-CPU sections (lines 98–106)

```
PROVIDE_HIDDEN (DSTK_TOP = MMAP_CPU_DSTT);     // 0xffffffffbfffe000
.cpulocal MMAP_CPU_DATA :                        // 0xffffffffbffff000
```

- `DSTK_TOP` = top of the per-CPU **data stack** (each CPU maps its own physical page here).
- `.cpulocal` is placed at `MMAP_CPU_DATA` — a per-CPU **virtual address** that each CPU maps to its own private physical page. It holds hot per-CPU data (`CPULOCAL` macro variables). The `SORT_BY_ALIGNMENT` ensures cache-friendly layout.
- The `ASSERT` ensures the entire per-CPU section fits in a single 4 KiB page.

---

## `/DISCARD/` (lines 108–113)

Strips ELF comments, notes, and exception-handling frames — none are needed in a freestanding microhypervisor.

---

## Summary: Memory Layout

```
Physical:      LOAD_ADDR ──────────────────────────────── NOVA_HPAE
               │.init│.text│.rodata│.init_array│.data│.bss│

Virtual:       LINK_ADDR (0xffffffffc0000000) ──────────▶
               │.init│      (same sections at high VA)     │
               ...
               MMAP_CPU_DATA (0xffffffffbffff000)  ← per-CPU page
               MMAP_CPU_DSTT (0xffffffffbfffe000)  ← per-CPU stack top
```

The `.init` section runs at physical addresses (before the MMU page tables are set up), then the kernel enables paging and everything else runs at virtual addresses near the top of the 64-bit address space.
