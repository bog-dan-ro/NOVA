/*
 * SBI (Supervisor Binary Interface)
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

#include "types.hpp"

class Sbi
{
    public:
        // SBI Extension IDs
        static constexpr unsigned long EXT_BASE         = 0x10;
        static constexpr unsigned long EXT_TIMER        = 0x54494D45;   // "TIME"
        static constexpr unsigned long EXT_IPI          = 0x735049;     // "sPI"
        static constexpr unsigned long EXT_RFENCE       = 0x52464E43;   // "RFNC"
        static constexpr unsigned long EXT_HSM          = 0x48534D;     // "HSM"
        static constexpr unsigned long EXT_SRST         = 0x53525354;   // "SRST"
        static constexpr unsigned long EXT_PMU          = 0x504D55;     // "PMU"

        // SBI Function IDs for Timer Extension
        static constexpr unsigned long TIMER_SET        = 0;

        // SBI Function IDs for IPI Extension
        static constexpr unsigned long IPI_SEND         = 0;

        // SBI Function IDs for RFENCE Extension
        static constexpr unsigned long RFENCE_I         = 0;
        static constexpr unsigned long RFENCE_VMA       = 1;
        static constexpr unsigned long RFENCE_VMA_ASID  = 2;
        static constexpr unsigned long RFENCE_GVMA     = 3;
        static constexpr unsigned long RFENCE_GVMA_VMID = 4;
        static constexpr unsigned long RFENCE_VVMA     = 5;
        static constexpr unsigned long RFENCE_VVMA_ASID = 6;

        // SBI Function IDs for HSM Extension
        static constexpr unsigned long HSM_START        = 0;
        static constexpr unsigned long HSM_STOP         = 1;
        static constexpr unsigned long HSM_STATUS       = 2;
        static constexpr unsigned long HSM_SUSPEND      = 3;

        // SBI Function IDs for SRST Extension
        static constexpr unsigned long SRST_RESET       = 0;

        // SBI Return Error Codes
        static constexpr long SUCCESS               = 0;
        static constexpr long ERR_FAILED            = -1;
        static constexpr long ERR_NOT_SUPPORTED     = -2;
        static constexpr long ERR_INVALID_PARAM     = -3;
        static constexpr long ERR_DENIED            = -4;
        static constexpr long ERR_INVALID_ADDRESS   = -5;
        static constexpr long ERR_ALREADY_AVAILABLE = -6;
        static constexpr long ERR_ALREADY_STARTED   = -7;
        static constexpr long ERR_ALREADY_STOPPED   = -8;

        struct Result {
            long error;
            long value;
        };

        // Generic SBI call
        static Result call (unsigned long ext, unsigned long fid,
                           unsigned long a0 = 0, unsigned long a1 = 0,
                           unsigned long a2 = 0, unsigned long a3 = 0,
                           unsigned long a4 = 0, unsigned long a5 = 0)
        {
            register unsigned long r_a0 asm ("a0") = a0;
            register unsigned long r_a1 asm ("a1") = a1;
            register unsigned long r_a2 asm ("a2") = a2;
            register unsigned long r_a3 asm ("a3") = a3;
            register unsigned long r_a4 asm ("a4") = a4;
            register unsigned long r_a5 asm ("a5") = a5;
            register unsigned long r_a6 asm ("a6") = fid;
            register unsigned long r_a7 asm ("a7") = ext;

            asm volatile ("ecall"
                          : "+r" (r_a0), "+r" (r_a1)
                          : "r" (r_a2), "r" (r_a3), "r" (r_a4), "r" (r_a5),
                            "r" (r_a6), "r" (r_a7)
                          : "memory");

            return { static_cast<long>(r_a0), static_cast<long>(r_a1) };
        }

        // Set timer
        static void set_timer (uint64_t stime)
        {
            call (EXT_TIMER, TIMER_SET, stime);
        }

        // Send IPI
        static void send_ipi (unsigned long hart_mask, unsigned long hart_mask_base = 0)
        {
            call (EXT_IPI, IPI_SEND, hart_mask, hart_mask_base);
        }

        // Remote fence.i
        static void remote_fence_i (unsigned long hart_mask, unsigned long hart_mask_base = 0)
        {
            call (EXT_RFENCE, RFENCE_I, hart_mask, hart_mask_base);
        }

        // Remote sfence.vma
        static void remote_sfence_vma (unsigned long hart_mask, unsigned long hart_mask_base,
                                       unsigned long start, unsigned long size)
        {
            call (EXT_RFENCE, RFENCE_VMA, hart_mask, hart_mask_base, start, size);
        }

        // Start hart
        static long hart_start (unsigned long hartid, unsigned long start_addr, unsigned long opaque)
        {
            return call (EXT_HSM, HSM_START, hartid, start_addr, opaque).error;
        }

        // Stop hart
        static long hart_stop()
        {
            return call (EXT_HSM, HSM_STOP).error;
        }

        // Get hart status
        static long hart_status (unsigned long hartid)
        {
            return call (EXT_HSM, HSM_STATUS, hartid).value;
        }

        // System reset
        static void system_reset (unsigned long reset_type, unsigned long reset_reason)
        {
            call (EXT_SRST, SRST_RESET, reset_type, reset_reason);
        }
};

