/*
 * Reference Counting
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
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

#include "assert.hpp"
#include "atomic.hpp"

/*
 * Base class providing atomic reference counting for kernel objects.
 *
 * The reference count starts at zero. The first reference is established with
 * ref_inc() (used immediately after construction). Additional references are
 * acquired with try_inc(), which fails if the object is already dead (count
 * zero) or the count would overflow. When the last ref_dec() call drives the
 * count back to zero, collect() followed by retire() are invoked.
 *
 * Kobject overrides retire() to submit the object to the RCU queue so that
 * destroy() is called only after a full RCU grace period.
 */
class Refcnt
{
    private:
        Atomic<size_t> ref { 0 };

        virtual void collect() = 0;

        // Called after collect() when the refcount reaches zero. Kobject overrides this
        // to submit itself to the RCU reclamation queue; non-Kobject users keep the no-op.
        virtual void retire() {}

    protected:
        // Constructor
        Refcnt() = default;

        // No copy/move for reference-counted objects
        Refcnt            (Refcnt const &) = delete;
        Refcnt& operator= (Refcnt const &) = delete;

        [[nodiscard]] bool dead() const { return ref == 0; }

    public:
        // Increment refcount unless zero or overflowing
        /*
         * Attempt to increment the reference count.
         *
         * Fails (returns 0) if the object is dead (count == 0) or the count
         * would overflow. This is the safe way to obtain a reference to an
         * object whose liveness is not guaranteed by the caller.
         *
         * @return  New reference count on success, 0 on failure
         */
        [[nodiscard]] size_t try_inc()
        {
            for (size_t o { ref }, n; n = o + 1, o && n; )
                if (ref.compare_exchange (o, n)) [[likely]]
                    return n;

            return 0;
        }

        /*
         * Increment the reference count unconditionally.
         *
         * Asserts that the count is currently zero (i.e. this is the first
         * reference being established on a freshly constructed object).
         */
        void ref_inc()
        {
            assert (ref == 0);

            ++ref;
        }

        /*
         * Decrement the reference count unconditionally.
         *
         * When the count reaches zero, calls collect() (object-specific
         * cleanup) and retire() (schedules RCU-deferred destruction for
         * Kobject subclasses, no-op otherwise).
         */
        void ref_dec()
        {
            assert (ref != 0);

            // Invoke callback function when refcount becomes zero
            if (--ref == 0) [[unlikely]] {
                collect();
                retire();
            }
        }
};

/*
 * RAII smart pointer that holds one reference on a Refcnt-derived object.
 *
 * Acquires the reference via try_inc() at construction time; if try_inc()
 * fails (object is dead), the pointer is stored as nullptr. Releases the
 * reference via ref_dec() at destruction. Move construction/assignment
 * transfer ownership without touching the reference count. Copy operations
 * are deleted.
 */
template<typename T> class Refptr final
{
    private:
        T* ptr;

        void init (T *p)
        {
            ptr = p && p->try_inc() ? p : nullptr;
        }

        void fini()
        {
            if (ptr) [[likely]]
                ptr->ref_dec();
        }

        void xfer (Refptr &r)
        {
            ptr = r.ptr;
            r.ptr = nullptr;
        }

    public:
        // Destructor
        ~Refptr() { fini(); }

        // Constructor
        explicit constexpr Refptr() : ptr { nullptr } {}

        // Constructor
        explicit Refptr (T *p) { init (p); }

        // Copy Constructor
        Refptr (Refptr const &) = delete;

        // Copy Assignment
        Refptr& operator= (Refptr const &) = delete;

        // Move Constructor
        Refptr (Refptr&& r) { xfer (r); }

        // Move Assignment
        Refptr& operator= (Refptr&& r)
        {
            if (this != &r) [[likely]] {
                fini();
                xfer (r);
            }

            return *this;
        }

        // Conversion
        operator T*() const { return ptr; }

        // Member Selection
        T* operator->() const { return ptr; }

        // Dereference
        T& operator*() const { return *ptr; }

        // Atomic Load
        ALWAYS_INLINE inline auto atomic_load() const { return __atomic_load_n (&ptr, __ATOMIC_ACQUIRE); }

        // Atomic Compare-Exchange
        ALWAYS_INLINE inline bool atomic_compare_exchange (T *&old, Refptr &r)
        {
            // If ptr matches old, then swap ptr and r.ptr
            if (__atomic_compare_exchange (&ptr, &old, &r.ptr, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)) [[likely]] {
                r.ptr = old;
                return true;
            }

            return false;
        }
};
