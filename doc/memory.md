# NOVA Microhypervisor — Physical Memory Management

> **Scope**: This document describes how the NOVA microhypervisor manages physical memory on AArch64: how the kernel allocates memory for itself, how it controls access to physical memory, and how it exposes the remaining memory to the root task.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Kernel Memory — Buddy Allocator](#2-kernel-memory--buddy-allocator)
3. [Access-Control Model — `Space_hst::nova`](#3-access-control-model--space_hstnova)
4. [MMIO Exclusions](#4-mmio-exclusions)
5. [MMIO Address Discovery](#5-mmio-address-discovery)
6. [Memory Delegation to the Root Task](#6-memory-delegation-to-the-root-task)
7. [Physical Memory Map Summary](#7-physical-memory-map-summary)
8. [Delegation Mechanism — `Space_mem::delegate()`](#8-delegation-mechanism--space_memdelegate)
9. [Key Files](#9-key-files)

---

## 1. Overview

NOVA follows a strict **capability-based** memory model. Physical memory is not handed to the root task in bulk. Instead:

1. The kernel reserves memory for itself (code, data, BSS, buddy heap).
2. The kernel builds an **authority page table** (`Space_hst::nova`) that defines which physical pages are delegatable.
3. Device MMIO regions claimed by the kernel are **excluded** from delegation.
4. The root task (and its children) can only access physical memory by **delegating** pages from `Space_hst::nova` via hypercalls.

---

## 2. Kernel Memory — Buddy Allocator

**Files**: [../src/buddy.cpp](../src/buddy.cpp), [../inc/buddy.hpp](../inc/buddy.hpp)

The buddy allocator provides kernel-internal page allocation. It does **not** manage all physical RAM — only the kernel's own heap region.

### Pool Boundaries

```cpp
// src/buddy.cpp — Buddy::init()
auto const virt { reinterpret_cast<uintptr_t>(&KMEM_HVAS) };
auto const size { reinterpret_cast<uintptr_t>(Kmem::phys_to_ptr (Multiboot::ea)) - virt };
```

| Boundary | Symbol | Description |
|---|---|---|
| Start | `KMEM_HVAS` | Linker-defined: end of BSS, start of kernel heap |
| End | `Multiboot::ea` (`NOVA_HPAE`) | Physical end of the NOVA image |

### Default Size

The linker script ([../src/hypervisor.ld](../src/hypervisor.ld)) places `KMEM_HVAS` then aligns to `+62 MiB`:

```
PROVIDE_HIDDEN (KMEM_HVAS = .);
. = ALIGN (ABSOLUTE (.) + 62M, PAGE_SIZE (1));
PROVIDE_HIDDEN (NOVA_HPAE = . - OFFSET);
```

So the default buddy pool is approximately **62 MiB** (rounded up to a 2 MiB boundary).

### Optional Extension via KMEM Multiboot Tag

If the bootloader passes a `MULTIBOOT_V2_INFO_KMEM` tag, `start.S` extends `NOVA_HPAE` ([../src/aarch64/start.S](../src/aarch64/start.S) lines 171–180):

```asm
ldr     x5, =MULTIBOOT_V2_INFO_KMEM
cmp     x4, x5
bne     .Lmb2_nxt_tag
mov     x5, #(MMAP_GLB_MMIO - LINK_ADDR)   // Max: 896 MiB on AArch64
sub     x4, x28, x25
sub     x5, x5, x4
ldr     x4, [x3, #8]
cmp     x4, x5
csel    x4, x4, x5, lo                      // min(requested, available)
bic     x4, x4, #OFFS_MASK (1)              // Align to 2 MiB
add     x28, x28, x4                        // Extend NOVA_HPAE
```

The maximum extension is capped at `MMAP_GLB_MMIO - LINK_ADDR` (896 MiB on AArch64). This only grows the kernel heap — it does not affect how much RAM is visible to userspace.

### What the Buddy Allocator Is Used For

- Kernel page tables (`Hptp`, `Nptp`, `Dptp`)
- Kernel objects via per-PD slab caches (`Ec`, `Sc`, `Pt`, `Sm`, `Dc`, `Pd`, spaces, FPU contexts)
- `Vmcb` structures (buddy-allocated, ≤ `PAGE_SIZE`)
- UTCB pages
- Temporary mappings

---

## 3. Access-Control Model — `Space_hst::nova`

**File**: [../src/aarch64/space_hst.cpp](../src/aarch64/space_hst.cpp)

`Space_hst::nova` is a **singleton** kernel-owned host memory space. Its stage-2 page table (Nptp) serves as the **authority** — the master list of which physical pages may be delegated to any user PD.

### Constructor

```cpp
Space_hst::Space_hst() : Space_mem { Kobject::Subtype::HST }
{
    Space_obj::nova.insert (Space_obj::Selector::NOVA_HST,
        Capability { this, std::to_underlying (Capability::Perm_sp::TAKE) });

    constexpr uintptr_t max_addr { selectors << PAGE_BITS };

    auto const s { min (max_addr, Kmem::sym_to_phys (&NOVA_HPAS)) };  // NOVA image start
    auto const e { min (max_addr, Multiboot::ea) };                     // NOVA image end

    access_ctrl (0, s, Paging::Permissions (Paging::U | Paging::API));
    access_ctrl (e, max_addr - e, Paging::Permissions (Paging::U | Paging::API));
}
```

This creates **two identity-mapped regions** with full delegatable permissions (`U | API` = user-accessible, R/W/XU/XS):

| Region | Physical Range | Permissions | Meaning |
|---|---|---|---|
| Below kernel | `[0 .. NOVA_HPAS)` | `U \| API` | Delegatable to user PDs |
| Above kernel | `[Multiboot::ea .. max_addr)` | `U \| API` | Delegatable to user PDs |
| **Kernel image** | `[NOVA_HPAS .. Multiboot::ea)` | **Not mapped** | Never delegatable |

The kernel image gap is simply never inserted into the page table, so no `lookup()` on it can succeed and no delegation can ever reach those pages.

### Virtual Memory Layout (AArch64)

From [../inc/aarch64/memory.hpp](../inc/aarch64/memory.hpp):

```
LINK_ADDR       = VIRT_ADDR(511, 0,   0, 0)    // 896 MiB region
MMAP_GLB_MMIO   = VIRT_ADDR(511, 0, 448, 0)    //  64 MiB MMIO window
```

`max_addr = selectors << PAGE_BITS` = `2^(Npt::ibits - PAGE_BITS) << PAGE_BITS` = `2^48` bytes (256 TiB), which covers the full 48-bit physical address space.

---

## 4. MMIO Exclusions

During boot, every hardware component the kernel claims calls `Space_hst::access_ctrl(phys, size, Paging::NONE)` to **overwrite** the delegatable region with `NONE` permissions. Since `Npt::page_attr()` returns 0 when no API bits are set, the PTE becomes unmapped — making those physical pages **undelegatable**.

| Caller | MMIO Region | File |
|---|---|---|
| `Gicd::init()` | GIC Distributor | [../src/aarch64/gicd.cpp](../src/aarch64/gicd.cpp) |
| `Gicr::init()` | GIC Redistributor | [../src/aarch64/gicr.cpp](../src/aarch64/gicr.cpp) |
| `Gicc::init()` | GIC CPU Interface | [../src/aarch64/gicc.cpp](../src/aarch64/gicc.cpp) |
| `Gich::init()` | GIC Hypervisor Interface | [../src/aarch64/gich.cpp](../src/aarch64/gich.cpp) |
| `Mmio::mmap()` | Generic MMIO (SMMU, UART, ITS, etc.) | [../inc/mmio.hpp](../inc/mmio.hpp) |
| `Console_mbuf` | Message buffer (mapped `U\|W\|R`, not `NONE`) | [../src/console_mbuf.cpp](../src/console_mbuf.cpp) |

**Special case**: The message buffer (`Console_mbuf`) is mapped with `U|W|R` (readable/writable by root, but not executable), so it **can** be delegated with limited permissions.

---

## 5. MMIO Address Discovery

Section 4 describes *how* MMIO regions are excluded from delegation. This section explains *how NOVA learns the physical addresses* of those MMIO regions in the first place. There are two platform-discovery paths — **ACPI** and **Board + FDT** — determined at **compile time** by the `BOARD=` build variable.

### Board Selection (Compile Time)

**File**: [../inc/aarch64/board.hpp](../inc/aarch64/board.hpp)

The build system defines `BOARD_<name>` (e.g. `BOARD_acpi`, `BOARD_qemu`, `BOARD_nvidia_xavier`), which selects one of ~20 board-specific headers via `#ifdef` chains. Each board header defines a `Board` struct that inherits from `Board_defaults` and overrides compile-time constants.

**File**: [../inc/aarch64/board_defaults.hpp](../inc/aarch64/board_defaults.hpp)

```cpp
struct Board_defaults
{
    static constexpr uint64_t spin_addr {};
    static constexpr struct Cpu { uint64_t id; }       cpu[1]     {};
    static constexpr struct Tmr { unsigned ppi, flg; }  tmr[2]     {};
    static constexpr struct Gic { uint64_t mmio; unsigned size; } gic[4] {};
    static constexpr struct Its { uint64_t mmio; }      its[1]     {};
    static constexpr struct Smmu_v2 { uint64_t mmio; /* ... */ }  smmu_v2[1] {};
    static constexpr struct Smmu_v3 { uint64_t mmio; /* ... */ }  smmu_v3[1] {};
    static constexpr struct Uart { Debug::Subtype type; uint64_t mmio; unsigned clock; } uart {};
};
```

All fields default to **zero**. A board header overrides only the devices present on its SoC.

Example — QEMU ([../inc/aarch64/board_qemu.hpp](../inc/aarch64/board_qemu.hpp)):

```cpp
struct Board : Board_defaults
{
    static constexpr Cpu cpu[4] { { 0x0 }, { 0x1 }, { 0x2 }, { 0x3 } };
    static constexpr Gic gic[4] { { 0x8000000, 0x10000 }, { 0x80a0000, 0xf60000 },
                                   { 0x8010000, 0x10000 }, { 0x8030000, 0x10000 } };
    static constexpr Uart uart  { Debug::Subtype::SERIAL_PL011, 0x9000000, 24'000'000 };
};
```

For the pure-ACPI board ([../inc/aarch64/board_acpi.hpp](../inc/aarch64/board_acpi.hpp)):

```cpp
struct Board : Board_defaults {};   // everything is zero — all discovery at runtime
```

### How Board Constants Reach Device Drivers

Each GIC component initializes its physical address from the board constant at file scope:

| Device | Default Address Source | File |
|---|---|---|
| GIC Distributor (`Gicd`) | `Board::gic[0].mmio` | [../inc/aarch64/gicd.hpp](../inc/aarch64/gicd.hpp) |
| GIC Redistributor (`Gicr`) | `Board::gic[1].mmio` | (via MADT/FDT enumeration) |
| GIC CPU Interface (`Gicc`) | `Board::gic[2].mmio` | [../inc/aarch64/gicc.hpp](../inc/aarch64/gicc.hpp) |
| GIC Hypervisor Intf (`Gich`) | `Board::gic[3].mmio` | [../inc/aarch64/gich.hpp](../inc/aarch64/gich.hpp) |
| SMMU v2 | `Board::smmu_v2[N].mmio` | [../inc/aarch64/smmu_v2.hpp](../inc/aarch64/smmu_v2.hpp) |
| SMMU v3 | `Board::smmu_v3[N].mmio` | (via ACPI IORT) |
| ITS | `Board::its[N].mmio` | (via FDT init) |
| UART | `Board::uart.mmio` | Console singleton (see below) |

For example, in [../inc/aarch64/gicd.hpp](../inc/aarch64/gicd.hpp):

```cpp
static inline constinit uint64_t phys { Board::gic[0].mmio };
```

This is a `constinit` (not `constexpr`) variable, meaning it starts with the board constant but **can be overwritten at runtime** by ACPI.

### Path 1: ACPI (Runtime Discovery)

When `BOARD=acpi`, all board constants are zero. NOVA discovers device addresses entirely from ACPI firmware tables at runtime:

| ACPI Table | Discovers | Overrides | File |
|---|---|---|---|
| **MADT** (Multiple APIC Description Table) | GIC Distributor, CPU Interface, Hypervisor Interface, Redistributors, CPUs | `Gicd::phys`, `Gicc::phys`, `Gich::phys`, GICR ranges | [../src/aarch64/acpi_table_madt.cpp](../src/aarch64/acpi_table_madt.cpp) |
| **IORT** (I/O Remapping Table) | SMMUv3 instances | Creates `Smmu_v3` objects via `Smmu_v3::setup()` | [../src/aarch64/acpi_table_iort.cpp](../src/aarch64/acpi_table_iort.cpp) |
| **SPCR** (Serial Port Console Redirection) | Debug UART | `Console::bind()` rebinds console to discovered UART | [../src/generic/acpi/acpi_table_spcr.cpp](../src/generic/acpi/acpi_table_spcr.cpp) |
| **DBG2** (Debug Port Table 2) | Debug port(s) | `Console::bind()` | [../src/generic/acpi/acpi_table_dbg2.cpp](../src/generic/acpi/acpi_table_dbg2.cpp) |
| **GTDT** (Generic Timer Description Table) | Timer interrupts | Timer PPI/flags | (generic ACPI) |

Example from MADT parsing ([../src/aarch64/acpi_table_madt.cpp](../src/aarch64/acpi_table_madt.cpp)):

```cpp
Gicd::phys = phys_gicd;     // Override board default with ACPI-discovered address
Gicc::phys = phys_gicc;
Gich::phys = phys_gich;
```

### Path 2: Board + FDT (Compile-Time + Boot-Time)

For non-ACPI boards (e.g. `BOARD=qemu`, `BOARD=nvidia_xavier`), the board header provides hardcoded MMIO addresses. The FDT (Flattened Device Tree) passed by the bootloader is used **only for tracing/debugging** — NOVA does **not** dynamically extract device addresses from FDT nodes.

**File**: [../src/aarch64/fdt.cpp](../src/aarch64/fdt.cpp)

The FDT init path uses board constants directly:

```cpp
// In Fdt::init()
for (auto const &b : Board::its)
    if (b.mmio)
        Gits::setup (b.mmio);

for (auto const &b : Board::smmu_v2)
    if (b.mmio)
        Smmu_v2::setup (b);

for (auto const &b : Board::smmu_v3)
    if (b.mmio)
        Smmu_v3::setup (b.mmio, { b.evt, b.glb, b.cmd, b.pri });
```

The FDT blob is parsed into a tree structure (`parse_subtree`) for tracing output, but all MMIO addresses come from the `Board::` constants.

### Console UART Discovery

Each UART driver creates a **global singleton** using the board UART constant. A compile-time boolean-multiply trick ensures only the matching driver's singleton gets a non-zero address:

```cpp
// src/aarch64/console_uart_pl011.cpp
Console_uart_pl011 Console_uart_pl011::singleton {
    (Board::uart.type == Debug::Subtype::SERIAL_PL011) * Board::uart.mmio,
    Board::uart.clock
};
```

If `Board::uart.type` doesn't match `SERIAL_PL011`, the address is `0` and the singleton remains dormant. On ACPI boards (where `Board::uart` is all-zero), the SPCR or DBG2 table later calls `Console::bind()` to activate the correct UART at the runtime-discovered address.

### Summary

| Discovery Path | Board Constant | Runtime Override | Used By |
|---|---|---|---|
| **ACPI** | All zero (`board_acpi.hpp`) | MADT, IORT, SPCR, DBG2, GTDT | ACPI-capable servers/platforms |
| **Board + FDT** | Hardcoded SoC addresses | None (FDT not used for addresses) | Embedded SoCs (QEMU, Xavier, RK3399, etc.) |

---

## 6. Memory Delegation to the Root Task

**File**: [../src/ec.cpp](../src/ec.cpp) — `Ec::create_root()`

### At Boot Time

The root task does **not** receive all memory automatically. The kernel maps only three things into the root's `Space_hst`:

1. **Root ELF segments** — `PT_LOAD` segments from the root image:
   ```cpp
   hst->delegate (&Space_hst::nova, phys >> PAGE_BITS, virt >> PAGE_BITS,
                   (o = aligned_order (size, phys, virt)) - PAGE_BITS, perm, Memattr::ram());
   ```
   Each page is looked up in `Space_hst::nova` and only delegated if the authority permits it.

2. **HIP page** — Hypervisor Information Page, mapped read-only:
   ```cpp
   hst->update (info_addr, Kmem::ptr_to_phys (Hip::hip), 0,
                Paging::Permissions (Paging::K | Paging::U | Paging::R), Memattr::ram());
   ```
   Note: The HIP is mapped with `Paging::K`, which makes it **non-re-delegatable** (see below).

3. **UTCB page** — Mapped during HST EC construction with `K|U|W|R`.

### At Runtime (Hypercalls)

The root task maps additional physical memory into its own (or a child's) `Space_hst` via the **delegate hypercall**. This always passes through the authority check in `Space_mem::delegate()`.

### What the HIP Tells the Root Task

The HIP ([../src/hip.cpp](../src/hip.cpp)) contains the physical address ranges the root task needs to know about:

| HIP Field | Value | Purpose |
|---|---|---|
| `nova_p_addr` | `NOVA_HPAS` | Kernel image start — root must avoid this range |
| `nova_e_addr` | `Multiboot::ea` | Kernel image end |
| `root_p_addr` | First root ELF page | Root image start |
| `root_e_addr` | Last root ELF page | Root image end |
| `mbuf_p_addr/e_addr` | Message buffer range | Console ring buffer |
| `acpi_rsdp_addr` | ACPI RSDP | For root to discover platform topology |
| `uefi_mmap_addr/size` | UEFI memory map | For root to discover RAM layout |

The root task uses the UEFI memory map (or ACPI/FDT) from the HIP to learn the actual physical RAM layout, then issues delegate hypercalls to map the memory it needs — minus the kernel's range and MMIO regions.

---

## 7. Physical Memory Map Summary

```
Physical Address Space (48-bit, 256 TiB)
┌─────────────────────────────────────────┐  0
│                                         │
│  Delegatable to root task               │  ← Space_hst::nova: U|API
│  (minus MMIO exclusions)                │
│                                         │
├─────────────────────────────────────────┤  NOVA_HPAS (Kmem::sym_to_phys(&NOVA_HPAS))
│                                         │
│  NOVA kernel image                      │  ← NOT MAPPED in Space_hst::nova
│  (code, rodata, data, BSS,              │
│   page tables, MHIP, buddy heap)        │
│                                         │
├─────────────────────────────────────────┤  Multiboot::ea (NOVA_HPAE)
│                                         │
│  Delegatable to root task               │  ← Space_hst::nova: U|API
│  (minus MMIO exclusions)                │
│                                         │
├ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤  Various MMIO regions
│  GIC (GICD/GICR/GICC/GICH)             │  ← Overwritten to NONE
│  SMMU                                   │  ← Overwritten to NONE
│  UART                                   │  ← Overwritten to NONE
│  ITS                                    │  ← Overwritten to NONE
│  (scattered across address space)       │
├ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┤
│  Console message buffer                 │  ← Mapped U|W|R (delegatable, no exec)
└─────────────────────────────────────────┘  max_addr (2^48)
```

---

## 8. Delegation Mechanism — `Space_mem::delegate()`

**File**: [../src/space_mem.cpp](../src/space_mem.cpp)

Every memory delegation (boot-time and runtime) goes through this function:

```cpp
template<typename T>
Status Space_mem<T>::delegate (Space_hst const *hst,
    unsigned long ssb, unsigned long dsb, unsigned ord,
    unsigned pmm, Memattr ma)
{
    for (auto src { ssb }, dst { dsb }; src < sse; src += BITN(o), dst += BITN(o)) {

        // Look up source page in the authority (Space_hst::nova)
        auto pm { Paging::Permissions (hst->lookup (s, p, o, a) & (Paging::K | Paging::U | pmm)) };

        // Kernel memory cannot be delegated
        if (pm & Paging::K)
            pm = Paging::NONE;

        // Install mapping in destination space
        static_cast<T *>(this)->update (d, p, o, pm, ma);
    }
}
```

### Enforcement Rules

| Condition | Result | Example |
|---|---|---|
| Page not mapped in `Space_hst::nova` | `lookup` returns 0 → no permissions → **unmapped** | NOVA kernel pages |
| Page mapped with `NONE` | No API bits → **unmapped** | Kernel-claimed MMIO |
| Page mapped with `K` flag | Permissions masked to `NONE` → **blocked** | HIP page, UTCB pages |
| Page mapped with `U \| API` | Full delegation permitted | Normal RAM |
| Page mapped with `U \| W \| R` | Delegation with R/W only (no exec) | Console mbuf |

This ensures that:
- **NOVA's own memory** is never exposed (not in the authority page table at all)
- **Kernel-claimed MMIO** is never delegatable (overwritten to `NONE`)
- **Kernel-mapped pages** (HIP, UTCBs) with the `K` flag cannot be re-delegated by the root task
- Only pages explicitly marked `U` in the authority can flow to user PDs

---

## 9. Key Files

| File | Role |
|---|---|
| [../src/buddy.cpp](../src/buddy.cpp) | Buddy allocator: kernel page pool init and alloc/free |
| [../inc/buddy.hpp](../inc/buddy.hpp) | Buddy allocator: data structures and interface |
| [../src/aarch64/space_hst.cpp](../src/aarch64/space_hst.cpp) | `Space_hst::nova` constructor: authority page table |
| [../inc/aarch64/space_hst.hpp](../inc/aarch64/space_hst.hpp) | `Space_hst` class with `access_ctrl`, `delegate`, `make_current` |
| [../src/space_mem.cpp](../src/space_mem.cpp) | `Space_mem::delegate()`: capability-checked page transfer |
| [../inc/space_mem.hpp](../inc/space_mem.hpp) | `Space_mem::access_ctrl()`: identity-map with permissions |
| [../src/ec.cpp](../src/ec.cpp) | `Ec::create_root()`: root ELF loading and HIP mapping |
| [../src/hip.cpp](../src/hip.cpp) | `Hip::build()`: populates HIP with memory ranges |
| [../src/hypervisor.ld](../src/hypervisor.ld) | Linker script: KMEM_HVAS, NOVA_HPAE symbols |
| [../src/aarch64/start.S](../src/aarch64/start.S) | Assembly: KMEM tag handling, NOVA_HPAE extension |
| [../inc/aarch64/memory.hpp](../inc/aarch64/memory.hpp) | Virtual memory layout constants |
| [../inc/paging.hpp](../inc/paging.hpp) | `Paging::Permissions` enum (R, W, XU, XS, API, U, K) |
| [../inc/aarch64/board.hpp](../inc/aarch64/board.hpp) | Board selection via `BOARD=` compile macro |
| [../inc/aarch64/board_defaults.hpp](../inc/aarch64/board_defaults.hpp) | `Board_defaults` struct: zero-initialized device constants |
| [../inc/aarch64/board_acpi.hpp](../inc/aarch64/board_acpi.hpp) | Pure-ACPI board (all zeros, runtime discovery) |
| [../src/aarch64/acpi_table_madt.cpp](../src/aarch64/acpi_table_madt.cpp) | ACPI MADT: GIC address discovery, CPU enumeration |
| [../src/aarch64/acpi_table_iort.cpp](../src/aarch64/acpi_table_iort.cpp) | ACPI IORT: SMMUv3 discovery |
| [../src/generic/acpi/acpi_table_spcr.cpp](../src/generic/acpi/acpi_table_spcr.cpp) | ACPI SPCR: runtime console UART discovery |
| [../src/generic/acpi/acpi_table_dbg2.cpp](../src/generic/acpi/acpi_table_dbg2.cpp) | ACPI DBG2: debug port discovery |
| [../src/aarch64/fdt.cpp](../src/aarch64/fdt.cpp) | FDT init: uses Board constants for ITS/SMMU setup |

---

*End of memory documentation.*
