/*
 * Semaphore (SM)
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

#include "ec.hpp"

/*
 * A semaphore (SM) implements a counting semaphore, optionally backed by a
 * hardware interrupt (subtype SM_INT).
 *
 * dn() (P / wait): if the counter is positive it is decremented and the
 *   caller continues immediately; otherwise the calling EC is queued and
 *   blocked. An optional absolute deadline arms a hypercall timeout that
 *   wakes the EC with Status::TIMEOUT on expiry.
 * up() (V / signal): if any ECs are waiting, the head is woken; otherwise
 *   the counter is incremented (up to ~0ULL).
 * timeout(): called by the timeout subsystem when the deadline expires;
 *   dequeues the EC and wakes it with Status::TIMEOUT.
 *
 * The SM_INT subtype additionally stores a platform interrupt handle (`ptr`)
 * and an interrupt identifier (`iid`) used to re-arm the interrupt source.
 */
class Sm final : public Kobject, private Queue<Ec>
{
    private:
        Refptr<Pd>    const pd;     // Owner PD
        uint64_t            cnt;    // Semaphore counter
        void *        const ptr;    // Interrupt handle (SM_INT only)
        iid_t         const iid;    // Interrupt identifier (SM_INT only)
        Spinlock            lock;

        explicit Sm (Refptr<Pd> &, uintptr_t, void *);

        void collect() override final;

    public:
        [[nodiscard]] static Sm *create (Status &, Pd *, uintptr_t, void *);

        void destroy() override final;

        auto get_ptr() const { assert (subtype == Kobject::Subtype::SM_INT); return ptr; }
        auto get_iid() const { assert (subtype == Kobject::Subtype::SM_INT); return iid; }

        [[nodiscard]] Status dn (Ec *const self, bool zero, uint64_t t)
        {
            {   Lock_guard <Spinlock> guard { lock };

                // Fast path if the counter is > 0
                if (cnt) [[likely]] {
                    cnt = zero ? 0 : cnt - 1;
                    return Status::SUCCESS;
                }

                // Prevent blocking on a dead SM
                if (dead()) [[unlikely]]
                    return Status::ABORTED;

                // Block EC
                self->block();

                enqueue_tail (self);
            }

            // Determine if a remote CPU has already unblocked the EC
            if (!self->block_sc()) [[unlikely]]
                return Status::SUCCESS;

            // Program timeout if applicable
            if (t) [[likely]]
                self->set_timeout (t, this);

            // Reschedule
            Scheduler::schedule (true);
        }

        Status up()
        {
            Ec *ec;

            {   Lock_guard <Spinlock> guard { lock };

                if (!(ec = dequeue_head())) {

                    if (cnt == ~0ULL) [[unlikely]]
                        return Status::OVRFLOW;

                    cnt++;

                    return Status::SUCCESS;
                }

                // The EC can now be activated again
                ec->unblock (Ec::sys_finish<Status::SUCCESS, true>, false);
            }

            ec->unblock_sc();

            return Status::SUCCESS;
        }

        NONNULL
        void timeout (Ec *const ec)
        {
            {   Lock_guard <Spinlock> guard { lock };

                if (!ec->blocked())
                    return;

                dequeue (ec);

                // The EC can now be activated again
                ec->unblock (Ec::sys_finish<Status::TIMEOUT>, true);
            }

            ec->unblock_sc();
        }
};
