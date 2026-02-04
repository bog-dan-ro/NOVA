/*
 * Startup Code
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

// RISC-V status register bits
#define SSTATUS_SIE     0x2

// PTE attributes
#define PTE_ATTR_V      (1 << 0)    // Valid
#define PTE_ATTR_R      (1 << 1)    // Read
#define PTE_ATTR_W      (1 << 2)    // Write
#define PTE_ATTR_X      (1 << 3)    // Execute
#define PTE_ATTR_U      (1 << 4)    // User
#define PTE_ATTR_G      (1 << 5)    // Global
#define PTE_ATTR_A      (1 << 6)    // Accessed
#define PTE_ATTR_D      (1 << 7)    // Dirty
#define PTE_RWX_ADV     (PTE_ATTR_V | PTE_ATTR_R | PTE_ATTR_W | PTE_ATTR_X | PTE_ATTR_A | PTE_ATTR_D)

// Memory layout (must match memory.hpp)
// Link address: 0xffffffffc0000000 (top of address space, -1GB)
#define LINK_ADDR_HI    0xffffffff
#define LINK_ADDR_LO    0xc0000000

.global                 __init_bsp

/*
 * Initialization Code
 */
.section .init

__init_bsp:
                        // a0 = hart id
                        // a1 = device tree blob pointer

                        // Disable interrupts
                        csrci   sstatus, SSTATUS_SIE

                        // Save boot parameters in s-regs (callee-saved)
                        mv      s0, a0                  // s0 = hart id
                        mv      s1, a1                  // s1 = dtb pointer

                        // Get our load address
                        la      s2, __init_bsp          // s2 = our physical address

                        // Build link address in s3
                        lui     s3, %hi(LINK_ADDR_LO)
                        addi    s3, s3, %lo(LINK_ADDR_LO)
                        li      t0, LINK_ADDR_HI
                        slli    t0, t0, 32
                        or      s3, s3, t0              // s3 = 0xffffffffc0000000

                        // Check load address alignment (2MB)
                        li      t0, 0x1fffff
                        and     t0, s2, t0
                        bnez    t0, .Ldead

                        // Zero BSS
                        la      t0, __bss_start
                        la      t1, __bss_end
1:                      bgeu    t0, t1, 2f
                        sd      zero, 0(t0)
                        addi    t0, t0, 8
                        j       1b
2:

                        // Set up initial page tables
                        // We create Sv39 page tables with:
                        //  - Identity mapping of our physical address (1GB superpage)
                        //  - Mapping of LINK_ADDR to our physical address

                        // Clear page table area (PT2H_HPAS)
                        la      t0, PT2H_HPAS
                        li      t1, 4096                // One page
                        add     t1, t0, t1
3:                      bgeu    t0, t1, 4f
                        sd      zero, 0(t0)
                        addi    t0, t0, 8
                        j       3b
4:

                        // Clear L1 page table (PT1H_HPAS)
                        la      t0, PT1H_HPAS
                        li      t1, 4096
                        add     t1, t0, t1
5:                      bgeu    t0, t1, 6f
                        sd      zero, 0(t0)
                        addi    t0, t0, 8
                        j       5b
6:

                        // Set up L2 (root) page table entry pointing to L1
                        // For LINK_ADDR = 0xffffffffc0000000, VPN[2] = 511
                        la      t0, PT2H_HPAS
                        la      t1, PT1H_HPAS
                        srli    t1, t1, 12              // Get PPN of L1
                        slli    t1, t1, 10              // Shift to PTE[53:10]
                        ori     t1, t1, PTE_ATTR_V      // Set valid bit
                        li      t2, 511 * 8             // Index 511, 8 bytes per entry
                        add     t0, t0, t2
                        sd      t1, 0(t0)

                        // Set up identity mapping (1GB superpage at our load address)
                        la      t0, PT2H_HPAS
                        mv      t1, s2                  // Physical address
                        srli    t1, t1, 30              // Get GB-aligned PPN
                        slli    t3, t1, 3               // Table offset (index * 8)
                        add     t0, t0, t3              // Entry address
                        slli    t1, t1, 28              // PPN for 1GB page (bits 53:28)
                        ori     t1, t1, PTE_RWX_ADV     // Add attributes
                        sd      t1, 0(t0)

                        // Set up L1 page table for LINK_ADDR -> physical
                        // For LINK_ADDR = 0xffffffffc0000000, VPN[1] = 0
                        // Map as 2MB superpage
                        la      t0, PT1H_HPAS
                        mv      t1, s2                  // Physical address
                        srli    t1, t1, 12              // Get PPN
                        slli    t1, t1, 10              // Shift to PTE position
                        ori     t1, t1, PTE_RWX_ADV     // Add attributes
                        sd      t1, 0(t0)               // Store at index 0

                        // Enable Sv39 paging
                        // satp = Mode[63:60] | ASID[59:44] | PPN[43:0]
                        // Mode 8 = Sv39
                        la      t0, PT2H_HPAS
                        srli    t0, t0, 12              // Get PPN
                        li      t1, 8                   // Sv39 mode
                        slli    t1, t1, 60              // Shift to bits 63:60
                        or      t0, t0, t1
                        csrw    satp, t0
                        sfence.vma

                        // Calculate offset for jump
                        la      t0, .Lvirt              // Physical address of .Lvirt
                        sub     s4, s3, s2              // s4 = Offset = LINK_ADDR - physical (save for later)
                        la      t2, __init_bsp          // Base of image
                        sub     t0, t0, t2              // Offset within image
                        add     t0, t0, s3              // Virtual address
                        jr      t0

.Lvirt:
                        // Now running at virtual address

                        // Store boot parameters
                        // la gives address computed as: actual_PC + pcrel_offset
                        // where pcrel_offset = symbol_vma - instruction_vma
                        // Actual PC is at virtual address (LINK_ADDR + x), but instruction VMA is physical
                        // So result = (LINK_ADDR + x) + (symbol_vma - physical_addr)
                        //           = (LINK_ADDR + x) + (symbol_vma - LOAD_ADDR - x)
                        //           = LINK_ADDR + symbol_vma - LOAD_ADDR
                        //           = symbol_vma + OFFSET
                        // We need symbol_vma, so subtract OFFSET
                        la      t0, boot_hartid
                        sub     t0, t0, s4
                        sd      s0, 0(t0)
                        la      t0, boot_dtb
                        sub     t0, t0, s4
                        sd      s1, 0(t0)

                        // Set up stack
                        la      sp, STACK
                        sub     sp, sp, s4
                        li      t0, 4096
                        add     sp, sp, t0

                        // Set up trap vector
                        la      t0, trap_vector
                        sub     t0, t0, s4
                        csrw    stvec, t0

                        // Call C++ initialization
                        la      t0, preinit
                        sub     t0, t0, s4
                        jalr    ra, t0, 0

                        // Call global constructors
                        la      t0, init
                        sub     t0, t0, s4
                        jalr    ra, t0, 0

                        // Start kernel main loop
                        la      t0, bootstrap
                        sub     t0, t0, s4
                        jalr    ra, t0, 0

.Ldead:
                        wfi
                        j       .Ldead

/*
 * Trap Vector
 */
.section .text
.balign 4
.global trap_vector

trap_vector:
                        csrr    t0, scause
                        csrr    t1, sepc
                        csrr    t2, stval
                        j       .Ldead

/*
 * Data Section
 */
.section .data
.global boot_hartid, boot_dtb

boot_hartid:
                        .quad   0

boot_dtb:
                        .quad   0


