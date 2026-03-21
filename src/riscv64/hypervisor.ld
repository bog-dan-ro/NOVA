/*
 * Linker Script for RISC-V 64-bit
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

OUTPUT_ARCH(BFD_ARCH)
OUTPUT_FORMAT(BFD_FORMAT)
ENTRY(__init_bsp)

PHDRS
{
    init PT_LOAD FLAGS (5);
    kern PT_LOAD FLAGS (6);
}

SECTIONS
{
    . = LOAD_ADDR;

    /* NOVA_HPAS: Start of hypervisor image in physical address space
     * We define it at LINK_ADDR (virtual) since that's where the kernel
     * code runs and needs to reference it. The actual physical address
     * is LOAD_ADDR. Kmem::sym_to_phys will handle the conversion. */
    PROVIDE_HIDDEN (NOVA_HPAS = LINK_ADDR);

    .init :
    {
        *(.init .init.*)
    } : init

    .text . + OFFSET : AT (ADDR (.text) - OFFSET)
    {
        *(.text.hot .text.hot.*)
        *(.text .text.* .gnu.linkonce.t.*)
    } : kern

    .rodata : AT (ADDR (.rodata) - OFFSET)
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    } : kern

    .init_array : AT (ADDR (.init_array) - OFFSET)
    {
        PROVIDE_HIDDEN (CTORS_L = .);
        KEEP (*(SORT_BY_INIT_PRIORITY (.init_array.65534) SORT_BY_INIT_PRIORITY (.ctors.00001)))
        PROVIDE_HIDDEN (CTORS_C = .);
        KEEP (*(SORT_BY_INIT_PRIORITY (.init_array.65533) SORT_BY_INIT_PRIORITY (.ctors.00002)))
        PROVIDE_HIDDEN (CTORS_S = .);
        KEEP (*(SORT_BY_INIT_PRIORITY (.init_array.*) SORT_BY_INIT_PRIORITY (.ctors.*)))
        KEEP (*(.init_array .ctors))
        PROVIDE_HIDDEN (CTORS_E = .);
    } : kern

    .data : AT (ADDR (.data) - OFFSET)
    {
        *(.data .data.* .gnu.linkonce.d.*)
    } : kern

    .bss : AT (ADDR (.bss) - OFFSET)
    {
        PROVIDE_HIDDEN (__bss_start = .);

        . = ALIGN(4K) + 4K;
        PROVIDE_HIDDEN (STACK = . - OFFSET);

        PROVIDE_HIDDEN (PTAB_HPAS = . - OFFSET);
        PROVIDE_HIDDEN (PT2H_HPAS = . - OFFSET); . += 4K;
        PROVIDE_HIDDEN (PT1H_HPAS = . - OFFSET); . += 4K;

        *(.bss .bss.* .gnu.linkonce.b.*)

        . = ALIGN(4K);

        PROVIDE_HIDDEN (KMEM_HVAS = .);

        . = ALIGN (ABSOLUTE (.) + 8M, PAGE_SIZE (1));

        PROVIDE_HIDDEN (__bss_end = .);

    } : kern

    PROVIDE_HIDDEN (NOVA_HPAE = . - OFFSET);

    PROVIDE_HIDDEN (DSTK_TOP = MMAP_CPU_DSTT);

    .cpulocal MMAP_CPU_DATA :
    {
        *(SORT_BY_ALIGNMENT (.cpulocal.hot))
        *(SORT_BY_ALIGNMENT (.cpulocal))
    }

    . = ASSERT (SIZEOF (.cpulocal) <= PAGE_SIZE (0), "CPU-Local section too large");

    /DISCARD/ :
    {
        *(.comment*)
        *(.note*)
        *(.eh_frame*)
    }
}
