/*
 * Hypercall Timeout
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

#include "timeout.hpp"

class Sm;

/*
 * Per-EC timeout for blocking hypercalls with an absolute deadline.
 *
 * Embedded in Ec. When trigger() fires, it calls Sm::timeout() on the
 * associated semaphore, which removes the EC from the semaphore's wait queue
 * and wakes it with Status::TIMEOUT.
 */
class Timeout_hypercall : public Timeout
{
    private:
        Sm *sm { nullptr };  // Semaphore on which the EC is waiting; set by Ec::set_timeout()

        void trigger() override final;

    public:
        void enqueue (uint64_t t, Sm *s) { sm = s; Timeout::enqueue (t); }   // Arm with deadline and SM
};
