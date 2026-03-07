# NOVA Microhypervisor — RCU & Capability Lifecycle

> **Scope**: This document describes how kernel objects (`Kobject`) are created, published via capabilities, reference-counted, and safely reclaimed through the Read-Copy Update (RCU) mechanism. It covers `Refcnt`, `Kobject`, `Capability`, `Space_obj`, and `Rcu` in depth, and explains every step from object allocation to slab-cache free.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Class Hierarchy](#2-class-hierarchy)
3. [Reference Counting — `Refcnt`](#3-reference-counting--refcnt)
4. [Capability Encoding — `Capability`](#4-capability-encoding--capability)
5. [Object Space — `Space_obj`](#5-object-space--space_obj)
6. [Full Object Lifecycle](#6-full-object-lifecycle)
7. [RCU Mechanism — `Rcu`](#7-rcu-mechanism--rcu)
8. [Destruction — `Kobject::destroy()`](#8-destruction--kobjectdestroy)
9. [Error-Path (Never-Published) Destruction](#9-error-path-never-published-destruction)
10. [Key Files](#10-key-files)

---

## 1. Overview

NOVA uses a **capability-based access control model**. Every kernel resource (protection domain, execution context, scheduling context, portal, semaphore, device context) is represented as a `Kobject`. User-land accesses these resources exclusively through unforgeable *capability* tokens stored in a per-PD *object space* (`Space_obj`).

The lifecycle of a `Kobject` is:

```
allocate → construct → publish (first capability) → [use via capabilities]
         → last capability retracted → collect() → retire() → rcu_submit()
         → RCU grace period → destroy() → slab free
```

RCU ensures that no CPU is mid-flight using a stale pointer to an object at the moment the slab memory is released.

---

## 2. Class Hierarchy

```
Refcnt          — intrusive reference counting; calls collect() + retire() on last drop
│
└── Kobject  ◄─── Rcu::Element  — virtual destroy(); rcu_submit() enqueues on CPU-local list
     │
     ├── Pd          (Protection Domain)
     ├── Ec          (Execution Context)      ← arch-specific subclass
     ├── Sc          (Scheduling Context)
     ├── Pt          (Portal)
     ├── Sm          (Semaphore)
     ├── Dc          (Device Context)
     └── Space_*     (sub-spaces: OBJ, HST, GST, DMA, PIO, MSR)
```

`Kobject` multiply-inherits from both `Refcnt` (reference counting) and `Rcu::Element` (RCU queue membership). The two roles are intentionally separate:

- `Refcnt` tracks how many live capabilities point to the object.
- `Rcu::Element` provides the deferred-free queue linkage.

---

## 3. Reference Counting — `Refcnt`

**File**: `inc/refcnt.hpp`

```
Atomic<size_t> ref { 0 }     — starts at zero
```

### `ref_inc()`
Unconditional increment from 0 → 1. Asserts `ref == 0` — used **only** for the very first publication of a kernel object (`Capability::publish()`). Starting from zero means an object that has never been published cannot be accidentally resurrected via `try_inc()`.

### `try_inc()`
CAS loop that increments only if `ref > 0`. Used when acquiring an additional reference to an already-live object (`Capability::acquire()`). Returns 0 if the object is already dead — the caller must treat the capability as null.

### `ref_dec()`
Decrements `ref`. When it reaches zero:
1. Calls the pure-virtual `collect()` — subclass-specific cleanup hook (currently traces to the debug console).
2. Calls the virtual `retire()` — in `Kobject` this calls `rcu_submit()`, queuing the object for deferred destruction. Non-`Kobject` `Refcnt` users keep the default no-op.

### `Refptr<T>`
A move-only RAII wrapper for reference-counted pointers. The constructor calls `try_inc()`; the destructor calls `ref_dec()`. Internal kernel objects store owning PD references as `Refptr<Pd>` to keep the owning PD alive as long as any object it created is alive.

---

## 4. Capability Encoding — `Capability`

**File**: `inc/capability.hpp`

A `Capability` is a single `uintptr_t` value:

```
 63                   6  5             0
 ┌──────────────────────┬──────────────┐
 │   Kobject pointer    │  permissions │
 └──────────────────────┴──────────────┘
```

`Kobject::alignment` is 64 bytes (`BIT(6)`), so the low 6 bits are always zero in a valid pointer. Those bits store permission flags (`pmask = alignment - 1`). A value of 0 is the *null capability* — no object and no permissions.

### Reference-management methods

| Method | When called | Effect |
|---|---|---|
| `publish()` | First insertion into an object space | `ref_inc()` on the object (0 → 1) |
| `retract()` | Failed first insertion (CAS collision) | `ref_dec()` on the object (1 → 0, triggers collect + retire) |
| `acquire()` | Delegating a copy of an existing capability | `try_inc()` — fails silently if the object is already dead |
| `release()` | Overwriting or clearing an existing slot | `ref_dec()` on the old object |

`publish()` and `retract()` are asymmetric to `acquire()` and `release()`: the first pair handles the transition to/from "at least one capability exists"; the second pair handles copying and removal of existing capabilities.

---

## 5. Object Space — `Space_obj`

**File**: `inc/space_obj.hpp`, `src/space_obj.cpp`

The object space is a multi-level table indexed by a *selector* (integer). Each leaf slot holds one `Capability` value. Slots are allocated lazily using pages from the owning PD's host space.

### `insert(sel, cap)` — first publication

```
walk(sel, alloc=true)   → allocate leaf page if missing → Status::MEM_CAP on failure
cap.publish()           → ref_inc() on the object (0 → 1)
CAS slot ← cap.val      → if slot was non-zero, slot was occupied:
    cap.retract()       → ref_dec() (1 → 0) → collect() + retire() → rcu_submit()
    return BAD_CAP
return SUCCESS
```

`insert()` is only used for **initial publication** (called by `Pd::create_*`). It enforces that each selector can only hold one published capability at a time.

### `update(sel, cap)` — overwrite / clear

Used during capability delegation (`delegate()`) and explicit updates. Atomically swaps the old value for the new one:

```
cap.acquire()           → try_inc() on new object; on failure write null capability instead
ptr->exchange_n(new)    → atomic swap
Capability{old}.release() → ref_dec() on old object
```

This is the normal **revocation path**: when a capability slot is overwritten with a null capability, `release()` → `ref_dec()` → if last ref → `collect()` + `retire()` → `rcu_submit()`.

### `delegate(src, ssb, dsb, ord, pmm)` — copy a range

Iterates selector-by-selector, calling `update()` on each destination slot. Each `update()` acquires a new reference on the source object and releases the reference on whatever was previously in the destination slot.

---

## 6. Full Object Lifecycle

### 6.1 Creation and publication

```
Pd::create_pt(s, obj, sel, ec, ip)
  │
  ├─ Pt::create(s, ec, ip)
  │    └─ new (ec->get_pd()->pt_cache) Pt { ... }
  │         ref = 0 initially
  │
  └─ obj->insert(sel, Capability{o, perms})
       ├─ walk(sel)     — allocate leaf page
       ├─ cap.publish() — ref: 0 → 1
       └─ CAS succeeds  — capability is now visible to all CPUs
            return SUCCESS
```

If `insert()` fails with `MEM_CAP` (page allocation failed), `publish()` was never called and ref is still 0. The caller calls `o->destroy()` directly to free the slab slot. If `insert()` fails with `BAD_CAP` (slot occupied), `retract()` was already called inside `insert()`, which drops ref back to 0 and triggers `retire()` → `rcu_submit()`. The caller must **not** call `o->destroy()` again.

### 6.2 In-use reference acquisition

When an EC portal-calls a PT, it calls `cap.acquire()` → `try_inc()`. If successful, ref ≥ 2 for the duration of the call. No CPU can free the object while this reference is held.

### 6.3 Last capability removed

When the final capability slot pointing to an object is overwritten with a null capability (via `update()`), the chain is:

```
Space_obj::update()
  └─ Capability{old}.release()
       └─ old_obj->ref_dec()
            ref: 1 → 0  ──► collect()   — per-subclass trace
                        ──► retire()    — Kobject override
                               └─ rcu_submit()
                                    └─ Rcu::next.enqueue(this)  (CPU-local)
```

The object is now **logically dead** (ref = 0, `try_inc()` returns 0) but its memory is still intact. Any CPU that read the object pointer from a capability slot just before the CAS can still safely dereference it — the RCU grace period guarantees their execution window ends before `destroy()` is called.

---

## 7. RCU Mechanism — `Rcu`

**Files**: `inc/rcu.hpp`, `src/rcu.cpp`

### Per-CPU state

Each CPU maintains three singly-linked lists of `Rcu::Element *` and two epoch counters, all declared `CPULOCAL`:

| Variable | Role |
|---|---|
| `next` | Objects submitted by `rcu_submit()` but not yet associated with an epoch |
| `curr` | Objects waiting for epoch `epoch_c` to complete |
| `done` | Objects whose grace period has elapsed; ready for `destroy()` |
| `epoch_l` | Global epoch number last seen by this CPU |
| `epoch_c` | Global epoch number associated with `curr` |

### Global state

| Variable | Role |
|---|---|
| `epoch` | Global monotone counter. Low 2 bits are the COMPLETED/REQUESTED state flags. Upper bits (`>> 2`) are the epoch generation. |
| `count` | Number of CPUs yet to report a quiescent state in the current epoch |

### Epoch lifecycle

```
rcu_submit() → CPU-local next.enqueue(obj)

Rcu::check()  [called from Timeout_budget::trigger() on each scheduling tick]
  │
  ├─ if epoch_l ≠ global generation → epoch_l = new; set Hazard::RCU on this CPU
  │
  ├─ if curr non-empty and complete(epoch, epoch_c) → done.append(curr)
  │
  ├─ if next non-empty and curr empty
  │    → curr.append(next)
  │    → epoch_c = generation + 1
  │    → set_state(REQUESTED)    ← requests a new epoch
  │
  └─ if done non-empty → handle_callbacks()
                           └─ for each element: e->destroy()

Rcu::quiet()  [called on each CPU at context-switch / kernel-exit]
  └─ --count == 0 → set_state(COMPLETED)  ← all CPUs have passed quiescent state
```

`set_state()` advances to a new epoch only once both `REQUESTED` and `COMPLETED` are set:

- `REQUESTED` is set by whichever CPU drains `next` into `curr`.
- `COMPLETED` is set by the last CPU to call `quiet()`.

When both bits are set, `count` is reloaded with `Cpu::count` and `epoch` is incremented (clearing both bits), starting the next round.

### Quiescent state detection

`Hazard::RCU` is a per-CPU flag. When a new epoch starts, every CPU has this flag raised. At context switch or kernel exit (in arch-specific `Ec_arch` dispatch loops), the hazard is checked and `Rcu::quiet()` is called, clearing the flag and decrementing `count`. Once all CPUs have checked in, the epoch is complete and callbacks move from `curr` to `done`.

---

## 8. Destruction — `Kobject::destroy()`

**Files**: `src/pt.cpp`, `src/sm.cpp`, `src/ec.cpp`, `src/sc.cpp`, `src/pd.cpp`, `src/dc.cpp`, `src/space_obj.cpp`, arch-specific `src/<arch>/space_*.cpp`

Every concrete `Kobject` subclass implements `destroy()` with the same pattern:

```cpp
void Pt::destroy()
{
    auto &cache { ec->get_pd()->pt_cache };

    this->~Pt();                    // explicit destructor — releases owned resources

    operator delete (this, cache);  // returns slab slot to the owning PD's cache
}
```

Each object type uses the slab cache of its **owning PD** (`ec->get_pd()->pt_cache` for PT, `pd->sm_cache` for SM, etc.). The PD is kept alive by the `Refptr<Pd>` held by every object (directly or transitively through its EC). Accessing the cache after the destructor runs is safe because the owning PD's reference count is decremented only as part of the same object's destruction chain.

`destroy()` is **only** called from `Rcu::handle_callbacks()` in the normal lifecycle. It is called directly (bypassing RCU) only from `Pd::create_*` error paths when the object was never published (see §9).

---

## 9. Error-Path (Never-Published) Destruction

When `Space_obj::insert()` fails with `Status::MEM_CAP` (the walk could not allocate a leaf page), `publish()` was never reached. The newly created object has `ref == 0` and `rcu_submit()` was never called. The `Pd::create_*` function calls `o->destroy()` directly in this case — no RCU delay is needed because the object's address was never written to any shared data structure and therefore no CPU can hold a stale pointer to it.

```
// In every Pd::create_X():
if ((s = obj->insert(sel, cap)) == Status::SUCCESS) [[likely]]
    return o;

if (s == Status::MEM_CAP)   // publish() never called → direct free is safe
    o->destroy();
// BAD_CAP: retract() already called → retire() → rcu_submit() → RCU will destroy
```

---

## 10. Key Files

| File | Role |
|---|---|
| `inc/refcnt.hpp` | `Refcnt` base class: `ref_dec()`, `collect()`, `retire()`, `Refptr<T>` |
| `inc/kobject.hpp` | `Kobject`: multiply inherits `Refcnt` + `Rcu::Element`; `retire()` override |
| `inc/rcu.hpp` | `Rcu` class: `Element`, per-CPU lists, epoch state machine |
| `src/rcu.cpp` | `Rcu::check()`, `Rcu::quiet()`, `Rcu::handle_callbacks()` |
| `inc/capability.hpp` | `Capability` encoding; `publish()`, `retract()`, `acquire()`, `release()` |
| `inc/space_obj.hpp` | `Space_obj` interface: `insert()`, `update()`, `delegate()`, `lookup()` |
| `src/space_obj.cpp` | `Space_obj` implementation: multi-level table, CAS-based slot management |
| `src/pd.cpp` | `Pd::create_*()` factories; error-path lifetime management |
| `src/pt.cpp`, `src/sm.cpp`, `src/ec.cpp`, `src/sc.cpp`, `src/dc.cpp` | Per-type `collect()` and `destroy()` implementations |
| `src/timeout_budget.cpp` | Calls `Rcu::check()` on each scheduling-tick budget expiry |
| `src/<arch>/ec_arch.cpp` | Calls `Rcu::quiet()` at kernel exit / context-switch |
