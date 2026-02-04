/*
 * Execution Context (EC) - Architecture-Specific
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

#include "ec_arch.hpp"
#include "cpu.hpp"
#include "event.hpp"
#include "fpu.hpp"
#include "hazard.hpp"
#include "rcu.hpp"
#include "space_hst.hpp"
#include "stdio.hpp"

void Ec::handle_hazard (unsigned h, cont_t func)
{
    if (h & Hazard::RCU)
        Rcu::quiet();

    if (h & (Hazard::ILLEGAL | Hazard::RECALL | Hazard::SLEEP | Hazard::SCHED)) [[unlikely]] {

        Cpu::preemption_point();

        if (Cpu::hazard & Hazard::SLEEP) {      // Reload
            cont = func;
            Cpu::fini();
        }

        if (Cpu::hazard & Hazard::SCHED) {      // Reload
            cont = func;
            Scheduler::schedule();
        }

        if (h & Hazard::ILLEGAL)
            kill ("Illegal execution state");

        if (regs.hazard & Hazard::RECALL) {     // Reload

            regs.hazard.clr (Hazard::RECALL);

            // No VCPU support for RISC-V, always use exception path
            exc_regs().set_ep (Event::hst_arch + Event::Selector::RECALL);
            send_msg<Ec_arch::ret_user_exception> (this);
        }
    }

    // Point of no return after checking all diversions: this EC will run

    if (h & Hazard::FPU) [[unlikely]]
        Cpu::hazard & Hazard::FPU ? Fpu::disable() : Fpu::enable();
}

void Ec::adjust_offset_ticks (uint64_t)
{
    // No VCPU support in RISC-V minimal implementation
    // This is used for virtual timer offset on ARM
}

void Ec_arch::ret_user_hypercall (Ec *const self)
{
    auto const h { (Cpu::hazard ^ self->regs.hazard) & (Hazard::ILLEGAL | Hazard::RECALL | Hazard::FPU | Hazard::RCU | Hazard::SLEEP | Hazard::SCHED) };
    if (h) [[unlikely]]
        self->handle_hazard (h, ret_user_hypercall);

    trace (TRACE_CONT, "EC:%p %s to M:%#x IP:%#lx SP:%#lx", static_cast<void *>(self), __func__, self->exc_regs().mode(), self->exc_regs().ip(), self->exc_regs().sp());

    self->regs.get_hst()->make_current();

    // Return to user mode via sret
    auto &r { self->exc_regs() };
    asm volatile (
        // Load all GPRs from the exception frame
        "ld x1,  0*8(%0);"   // ra
        "ld x2,  1*8(%0);"   // sp
        "ld x3,  2*8(%0);"   // gp
        "ld x4,  3*8(%0);"   // tp
        "ld x5,  4*8(%0);"   // t0
        "ld x6,  5*8(%0);"   // t1
        "ld x7,  6*8(%0);"   // t2
        "ld x8,  7*8(%0);"   // s0/fp
        "ld x9,  8*8(%0);"   // s1
        "ld x10, 9*8(%0);"   // a0
        "ld x11, 10*8(%0);"  // a1
        "ld x12, 11*8(%0);"  // a2
        "ld x13, 12*8(%0);"  // a3
        "ld x14, 13*8(%0);"  // a4
        "ld x15, 14*8(%0);"  // a5
        "ld x16, 15*8(%0);"  // a6
        "ld x17, 16*8(%0);"  // a7
        "ld x18, 17*8(%0);"  // s2
        "ld x19, 18*8(%0);"  // s3
        "ld x20, 19*8(%0);"  // s4
        "ld x21, 20*8(%0);"  // s5
        "ld x22, 21*8(%0);"  // s6
        "ld x23, 22*8(%0);"  // s7
        "ld x24, 23*8(%0);"  // s8
        "ld x25, 24*8(%0);"  // s9
        "ld x26, 25*8(%0);"  // s10
        "ld x27, 26*8(%0);"  // s11
        "ld x28, 27*8(%0);"  // t3
        "ld x29, 28*8(%0);"  // t4
        "ld x30, 29*8(%0);"  // t5
        "ld x31, 30*8(%0);"  // t6
        // Load CSRs
        "ld t0, 31*8(%0);"   // sepc
        "csrw sepc, t0;"
        "ld t0, 32*8(%0);"   // sstatus
        "csrw sstatus, t0;"
        "sret;"
        : : "r" (&r.sys.gpr[0]) : "memory"
    );

    UNREACHED;
}

void Ec_arch::ret_user_exception (Ec *const self)
{
    auto const h { (Cpu::hazard ^ self->regs.hazard) & (Hazard::ILLEGAL | Hazard::RECALL | Hazard::FPU | Hazard::RCU | Hazard::SLEEP | Hazard::SCHED) };
    if (h) [[unlikely]]
        self->handle_hazard (h, ret_user_exception);

    trace (TRACE_CONT, "EC:%p %s to M:%#x IP:%#lx SP:%#lx", static_cast<void *>(self), __func__, self->exc_regs().mode(), self->exc_regs().ip(), self->exc_regs().sp());

    self->regs.get_hst()->make_current();

    // Return to user mode via sret (same as hypercall return)
    auto &r { self->exc_regs() };
    asm volatile (
        "ld x1,  0*8(%0);"
        "ld x2,  1*8(%0);"
        "ld x3,  2*8(%0);"
        "ld x4,  3*8(%0);"
        "ld x5,  4*8(%0);"
        "ld x6,  5*8(%0);"
        "ld x7,  6*8(%0);"
        "ld x8,  7*8(%0);"
        "ld x9,  8*8(%0);"
        "ld x10, 9*8(%0);"
        "ld x11, 10*8(%0);"
        "ld x12, 11*8(%0);"
        "ld x13, 12*8(%0);"
        "ld x14, 13*8(%0);"
        "ld x15, 14*8(%0);"
        "ld x16, 15*8(%0);"
        "ld x17, 16*8(%0);"
        "ld x18, 17*8(%0);"
        "ld x19, 18*8(%0);"
        "ld x20, 19*8(%0);"
        "ld x21, 20*8(%0);"
        "ld x22, 21*8(%0);"
        "ld x23, 22*8(%0);"
        "ld x24, 23*8(%0);"
        "ld x25, 24*8(%0);"
        "ld x26, 25*8(%0);"
        "ld x27, 26*8(%0);"
        "ld x28, 27*8(%0);"
        "ld x29, 28*8(%0);"
        "ld x30, 29*8(%0);"
        "ld x31, 30*8(%0);"
        "ld t0, 31*8(%0);"
        "csrw sepc, t0;"
        "ld t0, 32*8(%0);"
        "csrw sstatus, t0;"
        "sret;"
        : : "r" (&r.sys.gpr[0]) : "memory"
    );

    UNREACHED;
}
