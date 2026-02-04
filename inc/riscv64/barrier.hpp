/*
 * Barriers
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

#include "std.hpp"

class Barrier final
{
    public:
        enum class Domain
        {
            SY  = 0,    // Full System (RISC-V fences are always system-wide)
        };

        enum class Type
        {
            LD  = 1,    // Loads
            ST  = 2,    // Stores
            ALL = 3,    // Loads and Stores
        };

        /*
         * Read Memory Barrier
         *
         * Ensures all prior loads complete before subsequent memory operations
         */
        static void rmb (Domain) { asm volatile ("fence ir, iorw" : : : "memory"); }

        /*
         * Write Memory Barrier
         *
         * Ensures all prior stores complete before subsequent stores
         */
        static void wmb (Domain) { asm volatile ("fence ow, ow" : : : "memory"); }

        /*
         * Full Memory Barrier
         *
         * Ensures all prior memory operations complete before subsequent memory operations
         */
        static void fmb (Domain) { asm volatile ("fence iorw, iorw" : : : "memory"); }

        /*
         * Read Synchronization Barrier
         *
         * Same as rmb for RISC-V
         */
        static void rsb (Domain d) { rmb (d); }

        /*
         * Write Synchronization Barrier
         *
         * Same as wmb for RISC-V
         */
        static void wsb (Domain d) { wmb (d); }

        /*
         * Full Synchronization Barrier
         *
         * Same as fmb for RISC-V
         */
        static void fsb (Domain d) { fmb (d); }

        /*
         * Instruction Synchronization Barrier
         *
         * Ensures instruction stream is synchronized
         */
        static void isb() { asm volatile ("fence.i" : : : "memory"); }

        /*
         * SFENCE.VMA - Supervisor Fence Virtual Memory
         *
         * Synchronizes updates to page tables
         */
        static void sfence_vma() { asm volatile ("sfence.vma" : : : "memory"); }

        static void sfence_vma (uintptr_t vaddr) { asm volatile ("sfence.vma %0, zero" : : "r" (vaddr) : "memory"); }

        static void sfence_vma (uintptr_t vaddr, unsigned asid) { asm volatile ("sfence.vma %0, %1" : : "r" (vaddr), "r" (asid) : "memory"); }
};
