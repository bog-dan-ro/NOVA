/*
 * Timeout
 *
 * Copyright (C) 2014 Udo Steinberg, FireEye, Inc.
 * Copyright (C) 2019-2026 Udo Steinberg, BlueRock Security, Inc.
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

#include "compiler.hpp"
#include "types.hpp"

/*
 * Per-CPU, sorted linked-list of absolute-deadline timers.
 *
 * Each Timeout instance is embedded in the object that owns it (no separate
 * allocation). enqueue(t) inserts the timeout in deadline order and reprograms
 * the hardware timer if the new deadline is earlier than the current head.
 * dequeue() removes the timeout and returns its deadline. check() fires all
 * expired timeouts by calling their virtual trigger() method.
 *
 * Subclasses: Timeout_budget (SC budget) and Timeout_hypercall (syscall
 * deadline).
 */
class Timeout
{
    private:
        uint64_t    time    { 0 };      // Absolute deadline in STC ticks
        Timeout *   prev    { nullptr };
        Timeout *   next    { nullptr };

        static inline constinit Timeout *list CPULOCAL { nullptr };

        virtual void trigger() = 0;

    public:
        // Enforce a constructor for CPU-local timeouts
        Timeout() {}

        void enqueue (uint64_t);    // Insert into per-CPU list; reprogram timer if needed
        uint64_t dequeue();         // Remove from list; return deadline

        static void check();        // Fire all expired timeouts (call trigger())
        static void sync();         // Reprogram hardware timer to earliest pending deadline
        static uint64_t idle();     // Return microseconds until next timeout (for CPU halt)
};
