/*
 * Scheduler
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
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

#include "queue.hpp"
#include "spinlock.hpp"

class Sc;

/*
 * Per-CPU, priority-based preemptive scheduler.
 *
 * Two queues per CPU:
 *   ready:   SCs whose EC is runnable on this CPU, ordered by priority.
 *   release: SCs that were unblocked by a remote CPU and need to be moved
 *            into the ready queue; protected by a spinlock because it is
 *            accessed concurrently from other CPUs.
 *
 * unblock(sc):  makes sc runnable; uses ready if local, release+RRQ IPI if
 *               the SC belongs to a different CPU.
 * requeue():    drains the release queue into the ready queue (called from
 *               the RRQ IPI handler).
 * schedule():   main scheduling loop; never returns. When `blocked` is false
 *               the current SC is re-enqueued before selecting the next one.
 */
class Scheduler final
{
    public:
        static constexpr auto priorities { 128 };

        static void unblock (Sc *);
        static void requeue();

        static auto get_current() { return current; }

        static void set_current (Sc *s) { current = s; }

        [[noreturn]] static void schedule (bool = false);

    private:
        /*
         * Per-CPU run queue with 128 priority levels.
         *
         * enqueue(sc, t): inserts sc at its priority level, refills its budget
         *                 if exhausted, and sets Hazard::SCHED if sc should
         *                 preempt the currently running SC.
         * dequeue(t):     removes and returns the highest-priority SC, updates
         *                 prio_top, and adjusts the EC's TSC offset.
         */
        class Ready final
        {
            private:
                Queue<Sc>   queue[priorities];
                unsigned    prio_top { 0 };

            public:
                void enqueue (Sc *, uint64_t);
                auto dequeue (uint64_t);
        };

        /*
         * Per-CPU queue for SCs released by remote CPUs.
         *
         * enqueue(sc): appends sc under the target CPU's lock and sends an
         *              RRQ IPI to trigger requeue() on that CPU.
         * dequeue():   removes and returns the head SC under the local lock.
         */
        class Release final
        {
            private:
                Queue<Sc>   queue;
                Spinlock    lock;

            public:
                void enqueue (Sc *);
                auto dequeue();
        };

        static Ready        ready       CPULOCAL;
        static Release      release     CPULOCAL;
        static Sc *         current     CPULOCAL;
};
