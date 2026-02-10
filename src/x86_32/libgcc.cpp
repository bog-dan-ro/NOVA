/*
 * Compiler Runtime Helpers: x86_32
 *
 * Provides __atomic_load_8/__atomic_store_8 for 64-bit atomics on i486,
 * and __udivdi3/__udivmoddi4 for 64-bit unsigned division.
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

#include "types.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

extern "C" {

/*
 * 64-bit atomic load/store for UP i486 (no cmpxchg8b).
 * Atomicity is ensured by disabling interrupts.
 */
uint64_t __atomic_load_8 (volatile void const *ptr, int)
{
    uint32_t flags;
    asm volatile ("pushfl; popl %0; cli" : "=r" (flags) : : "memory");
    auto val { *static_cast<volatile uint64_t const *>(ptr) };
    asm volatile ("pushl %0; popfl" : : "r" (flags) : "memory");
    return val;
}

void __atomic_store_8 (volatile void *ptr, uint64_t val, int)
{
    uint32_t flags;
    asm volatile ("pushfl; popl %0; cli" : "=r" (flags) : : "memory");
    *static_cast<volatile uint64_t *>(ptr) = val;
    asm volatile ("pushl %0; popfl" : : "r" (flags) : "memory");
}

/*
 * 64-bit unsigned division: shift-and-subtract algorithm
 */
uint64_t __udivmoddi4 (uint64_t num, uint64_t den, uint64_t *rem)
{
    uint64_t quot { 0 };

    if (!den) {
        if (rem) *rem = 0;
        return 0;
    }

    if (den > num) {
        if (rem) *rem = num;
        return 0;
    }

    unsigned shift { 0 };
    auto d { den };
    while (!(d & (uint64_t { 1 } << 63)) && d <= num) {
        d <<= 1;
        shift++;
    }

    if (d > num) {
        d >>= 1;
        shift--;
    }

    for (unsigned i { 0 }; i <= shift; i++) {
        quot <<= 1;
        if (num >= d) {
            num -= d;
            quot |= 1;
        }
        d >>= 1;
    }

    if (rem) *rem = num;

    return quot;
}

uint64_t __udivdi3 (uint64_t num, uint64_t den)
{
    return __udivmoddi4 (num, den, nullptr);
}

uint64_t __umoddi3 (uint64_t num, uint64_t den)
{
    uint64_t rem;
    __udivmoddi4 (num, den, &rem);
    return rem;
}

}

#pragma GCC diagnostic pop
