/*
 * Startup Code: x86_32
 *
 * 32-bit protected mode with 2-level paging for i486.
 * No long mode, no PAE, no TXT/MLE.
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

#include "arch.hpp"
#include "memory.hpp"
#include "multiboot.hpp"
#include "selectors.hpp"

#define PTE_ATTR_P      BIT (0)     // Present
#define PTE_ATTR_W      BIT (1)     // Writable
#define PTE_ATTR_U      BIT (2)     // User
#define PTE_ATTR_A      BIT (5)     // Accessed
#define PTE_ATTR_D      BIT (6)     // Dirty
#define PTE_ATTR_S      BIT (7)     // Superpage (4M page in PDE)
#define PTE_ATTR_G      BIT (8)     // Global

.global                 __init_bsp

/*
 * Macros
 */
.macro                  PTE_FIX L, PTAB, VIRT, PHYS, ATTR
                        lea     (\PHYS) + (\ATTR)(%ebp), %ecx
                        mov     %ecx, (\PTAB) + 4 * ((\VIRT) >> LEVL_BITS (\L) & (BIT (PTE_BPL) - 1))(%ebp)
.endm

.macro                  PTE_REL L, PTAB, VIRT, PHYS, ATTR
                        lea     (\PHYS) + (\ATTR)(%ebp), %ecx
                        lea     (\VIRT)(%ebp), %edx
                        shr     $LEVL_BITS (\L), %edx
                        and     $(BIT (PTE_BPL) - 1), %edx
                        mov     %ecx, (\PTAB)(%ebp, %edx, 4)
.endm

/*
 * Initialization Code
 */
.section .init

/*
 * 32-bit Startup Code: Bootstrap Processor (BSP)
 */
.code32

__init_bsp:             // Check if we launched via Multiboot v2
                        cmp     $MULTIBOOT_V2_LOADER, %eax
                        je      .Lvalid_launch

                        // Check if we launched via Multiboot v1
                        cmp     $MULTIBOOT_V1_LOADER, %eax
                        je      .Lvalid_launch

                        hlt
                        jmp     .
.Lvalid_launch:
                        // Determine relocation offset
                        xchg    %ebx, %esp
                        pop     %esi
                        call    .Lret_addr
.Lret_addr:             pop     %ebp
                        push    %esi
            dec %esi;   sub     $.Lret_addr, %ebp
                        xchg    %ebx, %esp

                        // Store boot parameters
                        mov     %eax, multiboot_p0 - OFFSET(%ebp)
                        mov     %ebx, multiboot_p1 - OFFSET(%ebp)

                        // Store image end address
                        lea     NOVA_HPAE(%ebp), %eax
                        mov     %eax, multiboot_ea - OFFSET(%ebp)

.Lmb2_launch:
                        // Multiboot v2 Launch
                        cmpl    $MULTIBOOT_V2_LOADER, multiboot_p0 - OFFSET(%ebp)
                        jne     .Lmb1_launch

                        // Prepare tag loop
                        lea     0x8(%ebx), %ecx     // Tag Ptr
                        add     0x0(%ebx), %ebx     // Tag End
.Lmb2_tag_loop:
                        cmpl    $MULTIBOOT_V2_INFO_END, (%ecx)
                        je      .Lmb_end

                        cmpl    $MULTIBOOT_V2_INFO_CMD, (%ecx)
                        jne     .Lmb2_not_cmd
                        lea     0x8(%ecx), %eax
                        mov     %eax, multiboot_cl - OFFSET(%ebp)
                        jmp     .Lmb2_nxt_tag
.Lmb2_not_cmd:
                        cmpl    $MULTIBOOT_V2_INFO_MOD_32, (%ecx)
                        jne     .Lmb2_not_mod
                        mov     multiboot_ra + 0 - OFFSET(%ebp), %edx
                        test    %edx, %edx
                        jnz     .Lmb2_nxt_tag
                        mov     0x8(%ecx), %eax
                        mov     %eax, multiboot_ra + 0 - OFFSET(%ebp)
                        movl    $0, multiboot_ra + 4 - OFFSET(%ebp)
                        jmp     .Lmb2_nxt_tag
.Lmb2_not_mod:
                        cmpl    $MULTIBOOT_V2_INFO_KMEM, (%ecx)
                        jne     .Lmb2_nxt_tag
                        mov     $(MMAP_GLB_UART - LINK_ADDR), %edx
                        sub     $(NOVA_HPAE - LOAD_ADDR), %edx
                        mov     0x8(%ecx), %eax
                        cmp     %edx, %eax
                        cmova   %edx, %eax
                        and     $~OFFS_MASK (1), %eax
                        add     %eax, multiboot_ea - OFFSET(%ebp)
.Lmb2_nxt_tag:
                        mov     0x4(%ecx), %eax
                        lea     0x7(%ecx,%eax), %ecx
                        and     $~7, %ecx
                        cmp     %ebx, %ecx
                        jb      .Lmb2_tag_loop
                        jmp     .Lmb_end

.Lmb1_launch:
                        // Multiboot v1 Launch
                        cmpl    $MULTIBOOT_V1_LOADER, multiboot_p0 - OFFSET(%ebp)
                        jne     .Lmb_end

                        testb   $BIT (MULTIBOOT_V1_INFO_CMD), (%ebx)
                        jz      .Lmb1_not_cmd
                        mov     16(%ebx), %eax
                        mov     %eax, multiboot_cl - OFFSET(%ebp)
.Lmb1_not_cmd:
                        testb   $BIT (MULTIBOOT_V1_INFO_MOD), (%ebx)
                        jz      .Lmb_end
                        mov     20(%ebx), %eax
                        test    %eax, %eax
                        jz      .Lmb_end
                        mov     24(%ebx), %eax
                        mov     (%eax), %eax
                        mov     %eax, multiboot_ra - OFFSET(%ebp)
.Lmb_end:
                        // Ensure flags are in a defined state
                        cli
                        cld

                        // Load relocated boot GDT
                        lea     __boot_gdt(%ebp), %eax
                        mov     %eax, __relo_pdb - OFFSET(%ebp)
                        lgdt    (__relo_pdl - OFFSET)(%ebp)

                        // Configure Page Tables

                        // 2-level paging: PDE -> PTE -> 4K page
                        // identity-map the load section (using 4K pages in a PT)
                        PTE_REL 1, PT1S_HPAS, NOVA_HPAS, PT0L_HPAS, PTE_ATTR_A | PTE_ATTR_U | PTE_ATTR_W | PTE_ATTR_P

                        // link section maps (using 4K pages): PDE entry for LINK_ADDR region
                        PTE_FIX 1, PT1S_HPAS, LINK_ADDR, PT0H_HPAS, PTE_ATTR_A | PTE_ATTR_U | PTE_ATTR_W | PTE_ATTR_P

                        // Fill identity-map PT (load section) with 4K pages
                        lea     NOVA_HPAS(%ebp), %edx
                        shr     $LEVL_BITS (0), %edx
                        and     $(BIT (PTE_BPL) - 1), %edx
                        lea     PT0L_HPAS(%ebp, %edx, 4), %edx
                        lea     NOVA_HPAS + (PTE_ATTR_D | PTE_ATTR_A | PTE_ATTR_W | PTE_ATTR_P)(%ebp), %ecx
                        mov     multiboot_ea - OFFSET(%ebp), %eax
.Lpte_id_loop:          mov     %ecx, (%edx)
                        add     $4, %edx
                        add     $PAGE_SIZE (0), %ecx
                        cmp     %eax, %ecx
                        jb      .Lpte_id_loop

                        // Fill link section PT with 4K pages mapped to physical
                        lea     PT0H_HPAS + 4 * (LINK_ADDR >> LEVL_BITS (0) & (BIT (PTE_BPL) - 1))(%ebp), %edx
                        lea     NOVA_HPAS + (PTE_ATTR_G | PTE_ATTR_D | PTE_ATTR_A | PTE_ATTR_W | PTE_ATTR_P)(%ebp), %ecx
                        mov     multiboot_ea - OFFSET(%ebp), %eax
.Lpte_lk_loop:          mov     %ecx, (%edx)
                        add     $4, %edx
                        add     $PAGE_SIZE (0), %ecx
                        cmp     %eax, %ecx
                        jb      .Lpte_lk_loop

                        // Initialize jump target
                        lea     __init_all(%ebp), %ebx
                        mov     %ebx,           __relo_jip - OFFSET(%ebp)
                        movw    $SEL_KERN_CODE, __relo_jcs - OFFSET(%ebp)

                        // Enable paging
                        lea     STACK(%ebp), %esp
                        lea     PT1S_HPAS(%ebp), %ecx
                        mov     $(CR0_PG | CR0_WP | CR0_NE | CR0_TS | CR0_MP | CR0_PE), %eax
                        mov     %ecx, %cr3
                        mov     %eax, %cr0

                        ljmp    *__relo_jip - OFFSET(%ebp)

/*
 * 32-bit Initialization (BSP)
 *
 * Runs at link address (OFFSET applied)
 */
.text

__init_all:             // Load proper segment selectors
                        mov     $SEL_KERN_DATA, %ecx
                        mov     %cx, %ss
                        mov     %cx, %ds
                        mov     %cx, %es
                        xor     %ecx, %ecx
                        mov     %cx, %fs
                        mov     %cx, %gs

                        // Set stack
                        lea     DSTK_TOP, %esp

                        // Call preinit
                        call    preinit

                        // Call kern_ptab_setup
                        call    kern_ptab_setup

                        // Switch to new page table
                        mov     %eax, %cr3

                        // Call init
                        call    init

                        // Call bootstrap
                        call    bootstrap

                        // Should not return
                        hlt
                        jmp     .

/*
 * Boot Data Structures
 */
.section .init

.balign                 8
__relo_pdl:             .word   .Lboot_gdt - __boot_gdt - 1
__relo_pdb:             .long   0
__relo_jip:             .long   0
__relo_jcs:             .word   0

.balign                 8
__boot_gdt:             .quad   0                                   // NULL
                        .quad   0x00cf9b000000ffff                  // KERN_CODE (32-bit flat)
                        .quad   0x00cf93000000ffff                  // KERN_DATA (32-bit flat)
                        .quad   0x00cff3000000ffff                  // USER_DATA (32-bit flat, DPL=3) (placeholder for boot)
                        .quad   0x00cffb000000ffff                  // USER_CODE (32-bit flat, DPL=3) (placeholder for boot)
.Lboot_gdt:
