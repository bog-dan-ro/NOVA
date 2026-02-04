/*
 * Virtual-Memory Layout
 *
 * Copyright (C) 2019-2025 Udo Steinberg, BlueRock Security, Inc.
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#pragma once

#include "alignment.hpp"
#include "board.hpp"

#ifndef __ASSEMBLER__
#include <cstdint>
#endif

#ifndef RAM_SIZE
// Physical RAM size used for static layout calculations when board code does not provide it.
#define RAM_SIZE        0x80000000              // Assume 2GiB populated
#endif

#ifndef RAM_BASE
// Physical load address fallback when no RAM base is specified by the board configuration.
#define LOAD_ADDR       0
#else
// Physical load address inside RAM when a board-specific RAM base is available.
#define LOAD_ADDR (RAM_BASE + 0x200000)
#endif

// RISC-V Sv39 page table configuration
// Number of index bits consumed by each Sv39 page-table level.
#define PTE_BPL         9
// Base page offset width (4 KiB pages).
#define PAGE_BITS       12
// Aggregate translated bit width up to level L.
#define LEVL_BITS(L)    ((L) * PTE_BPL + PAGE_BITS)
// Page size covered by level L.
#define PAGE_SIZE(L)    BITN (LEVL_BITS (L))
// Offset mask for addresses within a level-L mapping.
#define OFFS_MASK(L)    (PAGE_SIZE (L) - 1)

// Sv39: 3-level page table (levels 2, 1, 0)
// Compose a canonical Sv39 virtual address from L2/L1/L0 indices.
#define VIRT_ADDR(L2,L1,L0)  (VALN_SHIFT (L2, LEVL_BITS (2)) | VALN_SHIFT (L1, LEVL_BITS (1)) | VALN_SHIFT (L0, LEVL_BITS (0)))

// Sign extension for Sv39 (bit 38 -> bit 63)
// For C++, use arithmetic shift; for ASM, define explicit values
#ifndef __ASSEMBLER__
// Compose and sign-extend an Sv39 virtual address into a full machine address.
#define VIRT_ADDR_EXT(L2,L1,L0)  (static_cast<intptr_t>(VIRT_ADDR(L2,L1,L0) << 25) >> 25)
#else
// Assembly-friendly: explicit negative values for upper address space
// LINK_ADDR = 0xffffffffc0000000 (-1GB from top of address space)
#endif

// Global Area (upper half of address space for Sv39)
// Sv39 uses sign extension: virtual addresses 0x0000003fffffffff to 0xffffffc000000000 are valid
// Using explicit unsigned values for the upper portion of the canonical address space
// Per-CPU global region base in the upper canonical address space.
#define MMAP_GLB_CPUS   (0xffffffc000000000UL)   //   1G (262144 CPUs) @ VPN[2]=509
// Global PCIe configuration window anchor (shares the CPU global region base).
#define MMAP_GLB_PCIE   MMAP_GLB_CPUS
// Start address of the global PCI segment aperture.
#define MMAP_GLB_PCIS   (0xffffffc000000000UL - (256UL << 30))  // 256G before CPUS
// End marker for temporary remap window 1.
#define MMAP_TMP_RW1E   MMAP_GLB_PCIS
// Start marker for temporary remap window 1.
#define MMAP_TMP_RW1S   (MMAP_TMP_RW1E - (1UL << 30))           //   1G (Remap Window 1)
// End marker for temporary remap window 0.
#define MMAP_TMP_RW0E   MMAP_TMP_RW1S
// Start marker for temporary remap window 0.
#define MMAP_TMP_RW0S   (MMAP_TMP_RW0E - (1UL << 30))           //   1G (Remap Window 0)

#ifndef __ASSEMBLER__
// Upper canonical address space (sign-extended from bit 38)
// Use unsigned constants to avoid narrowing conversion issues
// Virtual base for global map slot 1.
#define MMAP_GLB_MAP1   (0xfffffffffa000000UL)   //   4M + gap  (VPN: 511, 500, 0)
// Virtual base for global map slot 0.
#define MMAP_GLB_MAP0   (0xfffffffff8000000UL)   //   4M + gap  (VPN: 511, 496, 0)
// Global mapping address of the platform interrupt controller.
#define MMAP_GLB_PLIC   (0xfffffffff4008000UL)   //  64K (PLIC) (VPN: 511, 488, 32)
// Global mapping base for UART MMIO.
#define MMAP_GLB_UART   (0xfffffffff0000000UL)   //  16M        (VPN: 511, 480, 0)
// Global MMIO mapping base for board/device registers.
#define MMAP_GLB_MMIO   (0xffffffffe0000000UL)   //  64M        (VPN: 511, 448, 0)
// Link-time virtual base where the hypervisor image is mapped.
#define LINK_ADDR       (0xffffffffc0000000UL)   // 896M        (VPN: 511, 0, 0)
#else
// Assembly: use explicit negative value for top of canonical address space
// LINK_ADDR = 0xffffffffc0000000 = -1073741824 (signed)
// Link-time virtual base where the hypervisor image is mapped (assembler constant form).
#define LINK_ADDR       0xffffffffc0000000
#endif

// CPU-Local Area (VPN[2]=510)
#ifndef __ASSEMBLER__
// Per-CPU data page virtual address.
#define MMAP_CPU_DATA   (0xffffffffbffff000UL)   //   4K (VPN: 510, 511, 511)
// Per-CPU data-stack top virtual address.
#define MMAP_CPU_DSTT   (0xffffffffbfffe000UL)   // Data Stack Top  (VPN: 510, 511, 510)
// Per-CPU data-stack base virtual address.
#define MMAP_CPU_DSTB   (0xffffffffbfffd000UL)   // Data Stack Base (VPN: 510, 511, 509)
// Per-CPU token page virtual address.
#define MMAP_CPU_DTKN   (0xffffffffbfffc000UL)   // Token           (VPN: 510, 511, 508)
// Per-CPU CLINT mapping base virtual address.
#define MMAP_CPU_CLINT  (0xffffffffbfc00000UL)   // CLINT           (VPN: 510, 511, 0)
#else
// Per-CPU data page virtual address (assembler constant form).
#define MMAP_CPU_DATA   0xffffffffbffff000
// Per-CPU data-stack top virtual address (assembler constant form).
#define MMAP_CPU_DSTT   0xffffffffbfffe000
// Per-CPU data-stack base virtual address (assembler constant form).
#define MMAP_CPU_DSTB   0xffffffffbfffd000
// Per-CPU token page virtual address (assembler constant form).
#define MMAP_CPU_DTKN   0xffffffffbfffc000
// Per-CPU CLINT mapping base virtual address (assembler constant form).
#define MMAP_CPU_CLINT  0xffffffffbfc00000
#endif

// Delta from image load address to link-time virtual address.
#define OFFSET          (LINK_ADDR - LOAD_ADDR)

