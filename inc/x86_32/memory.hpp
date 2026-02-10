/*
 * Virtual-Memory Layout: x86_32
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

#define LOAD_ADDR       ALIGNMENT_ADDR (ALIGNMENT_NOVA)

#define PTE_BPL         10
#define PAGE_BITS       12
#define LEVL_BITS(L)    ((L) * PTE_BPL + PAGE_BITS)
#define PAGE_SIZE(L)    BITN (LEVL_BITS (L))
#define OFFS_MASK(L)    (PAGE_SIZE (L) - 1)

// 32-bit linear address space: 2-level paging (PDE/PTE)
// Kernel lives in the upper 256M (0xf0000000 - 0xffffffff)

// Space-Local Area
#define MMAP_SPC_PIO_E  0xffc02000                              //   8K PIO bitmap end
#define MMAP_SPC_PIO    0xffc00000                              //   8K PIO bitmap
#define MMAP_SPC        0xffc00000                              //   4M

// CPU-Local Area
#define MMAP_CPU_DATA   0xffbff000                              //   4K
#define MMAP_CPU_DSTT   0xffbfe000                              // Data Stack Top
#define MMAP_CPU_DSTB   0xffbfd000                              // Data Stack Base
#define MMAP_CPU        0xffbfc000                              //  16K

// Global Area
#define MMAP_GLB_MAP1   0xff800000                              //   4M (PSE page)
#define MMAP_GLB_MAP0   0xff400000                              //   4M (PSE page)
#define MMAP_GLB_CPUS   MMAP_CPU_DATA                           //   4K (1 CPU)
#define MMAP_GLB_PCIE   MMAP_GLB_CPUS                           //   0  (No ECAM)
#define MMAP_GLB_PCIS   MMAP_GLB_CPUS                           //   0  (No ECAM)
#define MMAP_GLB_UART   0xffa00000                              //   2M
#define LINK_ADDR       0xf0000000                              // 256M

#define MMAP_TMP_RW0S   0xef000000                              //  16M (Remap Window 0)
#define MMAP_TMP_RW0E   0xf0000000
#define BASE_ADDR       MMAP_TMP_RW0E

#define OFFSET          (LINK_ADDR - LOAD_ADDR)
