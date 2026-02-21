# NOVA Microhypervisor — Class Documentation

> **Scope**: This document covers all classes found in the architecture-independent headers (`inc/`), the RISC-V 64-bit specific headers (`inc/riscv64/`), and the AArch64 specific headers (`inc/aarch64/`). Every member listed below was verified by reading the corresponding source file.

---

## Table of Contents

1. [Kernel Object Hierarchy](#1-kernel-object-hierarchy)
2. [Capability System](#2-capability-system)
3. [Protection Domain & Spaces](#3-protection-domain--spaces)
4. [Execution & Scheduling](#4-execution--scheduling)
5. [IPC Primitives](#5-ipc-primitives)
6. [Memory Management](#6-memory-management)
7. [Page Tables](#7-page-tables)
8. [Synchronisation Primitives](#8-synchronisation-primitives)
9. [Timeout Infrastructure](#9-timeout-infrastructure)
10. [Console & I/O](#10-console--io)
11. [Data Structures](#11-data-structures)
12. [Hardware Abstraction](#12-hardware-abstraction)
13. [Utility & Support Classes](#13-utility--support-classes)
14. [System-Call Interface Structures](#14-system-call-interface-structures)
15. [RISC-V Architecture-Specific Classes](#15-risc-v-architecture-specific-classes)
16. [AArch64 Architecture-Specific Classes](#16-aarch64-architecture-specific-classes)

---

## 1. Kernel Object Hierarchy

### `Refcnt` — Reference Counting Base
**File**: [inc/refcnt.hpp](../inc/refcnt.hpp)

Base class providing intrusive reference counting for kernel objects.

| Member | Kind | Description |
|---|---|---|
| `ref` | `Atomic<size_t>` (private) | Current reference count |
| `collect()` | pure virtual (private) | Callback invoked when refcount drops to zero |
| `dead()` | `[[nodiscard]] bool` (protected) | Returns `true` if refcount is zero |
| `try_inc()` | `[[nodiscard]] size_t` (public) | Atomically increments refcount unless zero; returns new count or 0 on failure |
| `ref_inc()` | `void` (public) | Unconditionally increments refcount (asserts it was zero — used for first publish) |
| `ref_dec()` | `void` (public) | Unconditionally decrements refcount; calls `collect()` when it reaches zero |

Copy/move constructors and assignment operators are deleted.

---

### `Refptr<T>` — Reference-Counted Smart Pointer
**File**: [inc/refcnt.hpp](../inc/refcnt.hpp)

RAII wrapper that acquires/releases a reference on a `Refcnt`-derived object.

| Member | Kind | Description |
|---|---|---|
| `ptr` | `T*` (private) | Raw pointer to the referenced object |
| `init(T* p)` | private | Acquires reference via `try_inc()` |
| `fini()` | private | Releases reference via `ref_dec()` |
| `xfer(Refptr& r)` | private | Moves ownership from `r` |
| `Refptr()` | constructor | Constructs a null reference |
| `Refptr(T* p)` | constructor | Constructs and acquires reference |
| `Refptr(Refptr&&)` | move constructor | Transfers ownership |
| `operator=(Refptr&&)` | move assignment | Releases old, transfers new |
| `operator T*()` | conversion | Implicit conversion to raw pointer |
| `operator->()` | `T*` | Member access |
| `operator*()` | `T&` | Dereference |
| `atomic_load()` | `auto` | Atomic load of the internal pointer (ACQUIRE) |
| `atomic_compare_exchange(T*&, Refptr&)` | `bool` | Atomic CAS on the pointer (SEQ_CST) |

Copy constructor and copy assignment are deleted.

---

### `Rcu` — Read-Copy Update
**File**: [inc/rcu.hpp](../inc/rcu.hpp)

Implements epoch-based RCU reclamation.

| Member | Kind | Description |
|---|---|---|
| `epoch` | `static Atomic<Epoch>` | Global epoch and state |
| `count` | `static Atomic<cpu_t>` | CPUs pending quiescent state in current epoch |
| `next` | `static List` (CPULOCAL) | Callbacks for a future epoch |
| `curr` | `static List` (CPULOCAL) | Callbacks for current epoch |
| `done` | `static List` (CPULOCAL) | Completed callbacks |
| `epoch_l` | `static Epoch` (CPULOCAL) | Epoch for which quiescent state will be reported |
| `epoch_c` | `static Epoch` (CPULOCAL) | Epoch for which current callbacks are handled |
| `quiet()` | static | Report quiescent state |
| `check()` | static | Check for completable callbacks |
| `set_state(State)` | static (private) | Set epoch state |
| `complete(Epoch, Epoch)` | static (private) | Check epoch completion |
| `handle_callbacks()` | static (private) | Process completed callbacks |

#### `Rcu::Element` (nested struct)
| Member | Kind | Description |
|---|---|---|
| `rcu_next` | `Element*` | Next element in RCU callback chain |
| `destroy()` | pure virtual | Object-specific destruction logic |
| `rcu_submit()` | void | Enqueues this element for RCU reclamation |

#### `Rcu::List` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `head` | `Element*` | Head of callback list |
| `tail` | `Element**` | Tail pointer |
| `clear()` | void | Resets the list |
| `append(List*)` | void | Appends another list |
| `enqueue(Element*)` | void | Adds an element to the tail |

---

### `Kobject` — Kernel Object Base
**File**: [inc/kobject.hpp](../inc/kobject.hpp)

Base class for all kernel objects. Inherits from `Refcnt` and `Rcu::Element`.

| Member | Kind | Description |
|---|---|---|
| `alignment` | `static constexpr auto` | Required alignment (`BIT(6)` = 64 bytes) |
| `type` | `Type const` (public) | Object type discriminator |
| `subtype` | `Subtype const` (public) | Object subtype discriminator |
| `Kobject(Type, Subtype)` | protected constructor | Initialises type and subtype |
| `operator new(size_t, Slab_cache&)` | `[[nodiscard]] static void*` (protected) | Allocates from a slab cache |
| `operator delete(void*, Slab_cache&)` | static (protected) | Frees to a slab cache |

#### `Kobject::Type` (enum class : uint8_t)
`PD`, `EC`, `SC`, `PT`, `SM`, `DC`

#### `Kobject::Subtype` (enum class : uint8_t)
`NONE`, `PD`, `OBJ`, `HST`, `GST`, `DMA`, `PIO`, `MSR`, `EC_LOCAL`, `EC_GLOBAL`, `EC_VCPU_REAL`, `EC_VCPU_OFFS`, `SM_REG`, `SM_INT`

---

## 2. Capability System

### `Capability` — Object Capability
**File**: [inc/capability.hpp](../inc/capability.hpp)

Wraps a `Kobject*` with permission bits stored in the low bits of the pointer (enabled by `Kobject::alignment`).

| Member | Kind | Description |
|---|---|---|
| `val` | `uintptr_t const` (private) | Encoded pointer + permission bits |
| `pmask` | `static constexpr uintptr_t` | Mask for permission bits (`Kobject::alignment - 1`) |
| `Capability(uintptr_t)` | constructor | Raw capability constructor |
| `Capability(Kobject*, unsigned)` | constructor | Object capability constructor |
| `obj()` | `Kobject*` | Extracts the object pointer (masks off permissions) |
| `prm()` | `unsigned` | Extracts the permission bits |
| `validate(Perm_sp, Subtype)` | `auto` | Validates a space capability |
| `validate(Perm_pd)` | `auto` | Validates a PD capability |
| `validate(Perm_ec)` | `auto` | Validates an EC capability |
| `validate(Perm_sc)` | `auto` | Validates an SC capability |
| `validate(Perm_pt)` | `auto` | Validates a PT capability |
| `validate(Perm_sm)` | `auto` | Validates an SM capability |
| `validate(Perm_sm, Subtype)` | `auto` | Validates an SM capability with subtype |
| `validate(Perm_dc)` | `auto` | Validates a DC capability |
| `validate_take_grant(...)` | `static bool` | Validates TAKE/GRANT permissions for delegation |
| `publish()` | `void` | Acquires first reference for an object (via `ref_inc`) |
| `retract()` | `void` | Releases first reference (via `ref_dec`) |
| `acquire()` | `bool` | Tries to acquire a reference (via `try_inc`) |
| `release()` | `void` | Releases a reference (via `ref_dec`) |

#### Permission Enums (all `enum class : unsigned`)
| Enum | Values |
|---|---|
| `Perm_sp` | `GRANT`, `TAKE`, `ASSIGN`, `DEFINED_OBJ`, `DEFINED_HST`, `DEFINED_GST`, `DEFINED_DMA`, `DEFINED_PIO`, `DEFINED_MSR` |
| `Perm_pd` | `PD`, `EC`, `SC`, `PT`, `SM`, `DC`, `DEFINED` |
| `Perm_ec` | `CTRL`, `BIND_PT`, `BIND_SC`, `DEFINED` |
| `Perm_sc` | `CTRL`, `DEFINED` |
| `Perm_pt` | `CTRL`, `CALL`, `EVENT`, `DEFINED` |
| `Perm_sm` | `CTRL_UP`, `CTRL_DN`, `ASSIGN`, `DEFINED`, `DEFINED_INT` |
| `Perm_dc` | `ASSIGN_DEV`, `ASSIGN_INT`, `DEFINED` |

---

## 3. Protection Domain & Spaces

### `Pd` — Protection Domain
**File**: [inc/pd.hpp](../inc/pd.hpp)

A protection domain owns slab caches and can create child objects and spaces. Inherits from `Kobject`.

| Member | Kind | Description |
|---|---|---|
| `pd` | `Refptr<Pd> const` (private) | Owner PD |
| `spaces` | `Atomic<unsigned>` (private) | Bitmask of attached space subtypes |
| `space_obj` | `Atomic<Space_obj*>` (private) | Object space |
| `space_hst` | `Atomic<Space_hst*>` (private) | Host (physical memory) space |
| `space_pio` | `Atomic<Space_pio*>` (private) | PIO space |
| `pd_cache` .. `fpu_cache` | `Slab_cache` (public) | Per-PD slab caches for PD, EC, SC, PT, SM, DC, OBJ, HST, GST, DMA, PIO, MSR, FPU |
| `nova` | `static Pd` | The root kernel PD |
| `root` | `static Pd*` | Pointer to the root user PD |
| `create(Status&, Pd*)` | `[[nodiscard]] static Pd*` | Factory method |
| `destroy()` | `void override final` | RCU-based destruction |
| `get_obj()` / `get_hst()` / `get_pio()` | getters | Return attached spaces |
| `create_dma/gst/hst/msr/obj/pio(...)` | `Space_*` factories | Create child spaces |
| `create_pd/ec/sc/pt/sm/dc(...)` | Object factories | Create child kernel objects |
| `attach(Subtype)` / `detach(Subtype)` | private | Atomically attach/detach a space subtype |
| `collect()` | `void override final` (private) | RCU collect callback |

---

### `Space` — Space Base Class
**File**: [inc/space.hpp](../inc/space.hpp)

Abstract base class for all address spaces. Inherits from `Kobject`.

| Member | Kind | Description |
|---|---|---|
| `pd` | `Refptr<Pd> const` (private) | Owner PD |
| `mco` | `static constexpr uint8_t` | Maximum capability order (default 0) |
| `sbw` | `static constexpr uint8_t` | Selector bit width (default 0) |
| `Space(Subtype)` | protected constructor | Constructor for kernel-owned spaces |
| `Space(Subtype, Refptr<Pd>&)` | protected constructor | Constructor for user-owned spaces |
| `get_pd()` | `Pd*` | Returns the owner PD |

---

### `Space_obj` — Object Space
**File**: [inc/space_obj.hpp](../inc/space_obj.hpp)

Manages capability selectors mapping to `Capability` objects. Uses a two-level page-table-like structure.

| Member | Kind | Description |
|---|---|---|
| `root` | `entry_t` (private) | Root of capability table |
| `nova` | `static Space_obj` | Kernel's own object space |
| `mco` | `static constexpr uint8_t` | Bits per level |
| `sbw` | `static constexpr uint8_t` | Total selector bit width (`bpl * lev`) |
| `selectors` | `static constexpr auto` | Maximum number of capability selectors |
| `create(Status&, Pd*)` | `[[nodiscard]] static Space_obj*` | Factory method |
| `destroy()` | `void override final` | Destructor |
| `lookup(unsigned long)` | `Capability` | Looks up a capability by selector |
| `update(unsigned long, Capability)` | `Status` | Updates a capability at selector |
| `insert(unsigned long, Capability)` | `Status` | Inserts a capability at selector |
| `delegate(...)` | `Status` | Delegates capabilities between object spaces |
| `walk(unsigned long, bool)` | `entry_t*` (private) | Walks the capability table |

#### `Space_obj::Selector` (enum)
`NOVA_CON`, `NOVA_OBJ`, `NOVA_HST`, `NOVA_PIO`, `NOVA_MSR`, `ROOT_OBJ`, `ROOT_HST`, `ROOT_PIO`, `ROOT_PD`, `NOVA_CPU`

#### `Space_obj::Captable` (nested struct, private)
| Member | Kind | Description |
|---|---|---|
| `entries` | `static constexpr auto` | Number of entries per table page |
| `slot[]` | `entry_t` | Array of capability entries |
| `operator new(size_t)` | `[[nodiscard]] static void*` | Allocates via `Buddy::alloc(0)` |
| `operator delete(void*)` | static | Frees via `Buddy::free` |
| `deallocate(unsigned)` | `void` | Recursively deallocates the subtree |

---

### `Space_mem<T>` — Memory Space (Template)
**File**: [inc/space_mem.hpp](../inc/space_mem.hpp)

Template base class for memory-backed spaces (HST, GST, DMA). Inherits from `Space`.

| Member | Kind | Description |
|---|---|---|
| `access_ctrl(T&, uint64_t, size_t, Permissions, Memattr)` | `static void` (protected) | Configures a range of physical memory attributes |
| `delegate(Space_hst const*, ...)` | `[[nodiscard]] Status` | Delegates memory mappings |

---

### `Space_hst` — Host Memory Space (RISC-V)
**File**: [inc/riscv64/space_hst.hpp](../inc/riscv64/space_hst.hpp)

Manages host-physical → virtual address translations using Sv39 page tables.

| Member | Kind | Description |
|---|---|---|
| `hptp` | `Hptp` (private) | Page table pointer |
| `nova` | `static Space_hst` | Kernel's host memory space |
| `sbw` | `static constexpr uint8_t` | Selector bit width (`Hpt::ibits - PAGE_BITS`) |
| `selectors` | `static constexpr uint64_t` | Maximum selectors |
| `mco()` | `static auto` | Maximum capability order |
| `create(Status&, Pd*)` | `[[nodiscard]] static Space_hst*` | Factory |
| `destroy()` | `void override final` | Destructor |
| `lookup(...)` | `auto` | Looks up a virtual address in the page table |
| `update(...)` | `auto` | Updates a page table entry |
| `sync()` | `void` | Invalidates TLB |
| `make_current()` | `void` | Activates this space's page table |
| `access_ctrl(uint64_t, size_t, Permissions)` | `static void` | Controls physical memory access |

---

### `Space_gst` — Guest Memory Space (RISC-V stub)
**File**: [inc/riscv64/space_gst.hpp](../inc/riscv64/space_gst.hpp)

Stub for guest memory space (no H-extension support in this configuration).

| Member | Kind | Description |
|---|---|---|
| `sbw` / `selectors` | `static constexpr` | Both zero (unsupported) |
| `create(...)` | always returns `nullptr` with `Status::BAD_FTR` |
| `update(...)` | always returns `Status::BAD_FTR` |

---

### `Space_dma` — DMA Memory Space (RISC-V stub)
**File**: [inc/riscv64/space_dma.hpp](../inc/riscv64/space_dma.hpp)

Stub for DMA (IOMMU) space. All operations return `Status::BAD_FTR`.

---

### `Space_pio` — PIO Space (RISC-V stub)
**File**: [inc/riscv64/space_pio.hpp](../inc/riscv64/space_pio.hpp)

Stub for port I/O space (RISC-V has no port I/O). All operations return `Status::BAD_FTR`.

---

### `Space_msr` — MSR Space (RISC-V stub)
**File**: [inc/riscv64/space_msr.hpp](../inc/riscv64/space_msr.hpp)

Stub for MSR space (RISC-V has CSRs, not MSRs). All operations return `Status::BAD_FTR`.

---

## 4. Execution & Scheduling

### `Ec` — Execution Context
**File**: [inc/ec.hpp](../inc/ec.hpp)

Represents a thread of execution. Inherits from `Kobject`, `Timeout_hypercall`, `Queue<Ec>::Element`, and (privately) `Queue<Sc>`.

| Member | Kind | Description |
|---|---|---|
| `cont_t` | `using` (private) | `void (*)(Ec*)` — Continuation function type |
| `regs` | `Cpu_regs` (private) | CPU register state |
| `evt` | `uintptr_t const` (private) | Event base selector |
| `cpu` | `cpu_t const` (private) | CPU affinity |
| `fpu` | `Fpu* const` (private) | FPU context pointer |
| `kpage` | `void* const` (private) | Kernel page (UTCB) |
| `callee` / `caller` | `Ec*` (private) | IPC partner chain |
| `cont` | `Atomic<cont_t>` (private) | Current continuation |
| `lock` | `Spinlock` (private) | Per-EC lock |
| `current` | `static Atomic<Ec*>` (CPULOCAL) | Currently running EC |
| `fpowner` | `static Ec*` (CPULOCAL) | Current FPU owner |
| `donations` | `static unsigned` (CPULOCAL) | IPC donation depth |
| `cpu_regs()` / `exc_regs()` / `sys_regs()` | private accessors | Access register sub-structures |
| `is_vcpu()` | `bool` (private) | True if subtype ≥ `EC_VCPU_REAL` |
| `get_utcb()` | `Utcb*` (private) | Returns the UTCB for this EC |
| `set_partner(Ec*)` / `clr_partner()` | private | Manage IPC partner relationships |
| `fpu_load()` / `fpu_save()` | private | FPU context switching |
| `handle_hazard(unsigned, cont_t)` | private | Process pending hazards |
| `help(Ec*, cont_t)` | private | Priority donation helper |
| `rendezvous(...)` | private | IPC rendezvous logic |
| `reply(cont_t)` | `[[noreturn]]` (private) | Reply from IPC |
| `kill(char const*)` | `[[noreturn]]` (private) | Kill the EC |
| `dead(Ec*)` / `blocking(Ec*)` / `idle(Ec*)` | `[[noreturn]] static` (private) | Sentinel continuations |
| `recv_kern(Ec*)` / `recv_user(Ec*)` | `[[noreturn]] static` (private) | Receive message handlers |
| `send_msg<cont_t>(Ec*)` | `[[noreturn]] static` (private) | Send message template |
| `sys_ipc_call(Ec*)` / `sys_ipc_reply(Ec*)` | `[[noreturn]] static` (private) | IPC system calls |
| `sys_create_pd/ec/sc/pt/sm/dc(Ec*)` | `[[noreturn]] static` (private) | Creation system calls |
| `sys_ctrl_pd/ec/sc/pt/sm/hw(Ec*)` | `[[noreturn]] static` (private) | Control system calls |
| `sys_assign_dev/int(Ec*)` | `[[noreturn]] static` (private) | Assignment system calls |
| `sys_finish_status(Status)` | `[[noreturn]]` (private) | Finish a system call with status |
| `get_pd()` | `Pd*` (public) | Returns the owning PD (via `regs.obj`) |
| `create(cpu_t, cont_t)` | `[[nodiscard]] static Ec*` | Factory: Kernel thread |
| `create_hst(Status&, ...)` | `[[nodiscard]] static Ec*` | Factory: Host EC |
| `create_gst(Status&, ...)` | `[[nodiscard]] static Ec*` | Factory: Guest EC |
| `destroy()` | `void override final` | RCU destruction |
| `create_idle()` / `create_root()` | `static void` | Create idle/root ECs |
| `switch_fpu(Ec*)` | `static bool` | Switch FPU ownership |
| `remote_current(cpu_t)` | `static Ec*` | Get current EC on another CPU |
| `block()` | `void` | Mark EC as blocked |
| `unblock(cont_t, bool)` | `void` | Mark EC as unblocked |
| `blocked()` | `bool` | Check if EC is blocked |
| `block_sc()` | `[[nodiscard]] bool` | Block associated SC (under lock) |
| `unblock_sc()` | `void` | Unblock all blocked SCs |
| `set_timeout(uint64_t, Sm*)` / `clr_timeout()` | void | Timeout management |
| `activate()` | `void` | Activate the EC |
| `adjust_offset_ticks(uint64_t)` | `void` | Adjust time offset |
| `sys_finish<Status, bool>()` | `[[noreturn]] static` | System call finish template |
| `syscall[16]` | `static constexpr cont_t[]` | System call dispatch table |

---

### `Ec_arch` — Architecture-Specific EC (RISC-V)
**File**: [inc/riscv64/ec_arch.hpp](../inc/riscv64/ec_arch.hpp)

RISC-V specific part of the EC. Privately inherits from `Ec`.

| Member | Kind | Description |
|---|---|---|
| `needs_pio` | `static constexpr auto` | `false` (no PIO on RISC-V) |
| `handle_irq_kern()` | `static void` | Kernel interrupt handler |
| `handle_irq_user()` | `[[noreturn]] static void` | User interrupt handler |
| `handle_exc_kern(Exc_regs*)` | `[[noreturn]] static void` | Kernel exception handler |
| `handle_exc_user(Exc_regs*)` | `[[noreturn]] static void` | User exception handler |
| `ret_user_hypercall(Ec*)` | `[[noreturn]] static void` | Return to user after hypercall |
| `ret_user_exception(Ec*)` | `[[noreturn]] static void` | Return to user after exception |
| `state_load(Ec*, Mtd_arch)` | `void` | Load UTCB state into CPU regs |
| `state_save(Ec*, Mtd_arch)` | `bool` | Save CPU regs into UTCB state |
| `make_current()` | `[[noreturn]]` | Switch to this EC (resets stack, invokes continuation) |
| `collect()` | `void override final` | RCU collect callback |

---

### `Sc` — Scheduling Context
**File**: [inc/sc.hpp](../inc/sc.hpp)

Represents a time quantum bound to an EC. Inherits from `Kobject` and `Queue<Sc>::Element`.

| Member | Kind | Description |
|---|---|---|
| `ec` | `Refptr<Ec> const` (private) | Bound EC |
| `budget` | `uint64_t const` (private) | Time budget (ticks) |
| `cpu` | `cpu_t const` (private) | CPU affinity |
| `cos` | `cos_t const` (private) | Class of service |
| `prio` | `uint8_t const` (private) | Priority |
| `used` | `Atomic<uint64_t>` (private) | Time used |
| `left` | `uint64_t` (private) | Time remaining in current period |
| `last` | `uint64_t` (private) | Last activation time |
| `create(Status&, Ec*, cpu_t, uint16_t, uint8_t, cos_t)` | `[[nodiscard]] static Sc*` | Factory |
| `destroy()` | `void override final` | Destructor |
| `get_ec()` | `Ec*` | Returns bound EC |
| `get_used()` | `uint64_t` | Returns time used |
| `collect()` | `void override final` (private) | RCU collect callback |

---

### `Scheduler` — CPU Scheduler
**File**: [inc/scheduler.hpp](../inc/scheduler.hpp)

| Member | Kind | Description |
|---|---|---|
| `priorities` | `static constexpr auto` | Number of priority levels (128) |
| `unblock(Sc*)` | `static void` | Enqueue an SC into the ready queue |
| `requeue()` | `static void` | Re-enqueue the current SC |
| `get_current()` | `static auto` | Returns the current SC |
| `set_current(Sc*)` | `static void` | Sets the current SC |
| `schedule(bool)` | `[[noreturn]] static void` | Main scheduler entry point |
| `ready` | `static Ready` (CPULOCAL, private) | Ready queue |
| `release` | `static Release` (CPULOCAL, private) | Release queue |
| `current` | `static Sc*` (CPULOCAL, private) | Currently running SC |

#### `Scheduler::Ready` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `queue[priorities]` | `Queue<Sc>` | Per-priority queues |
| `prio_top` | `unsigned` | Highest non-empty priority |
| `enqueue(Sc*, uint64_t)` | `void` | Enqueue an SC |
| `dequeue(uint64_t)` | `auto` | Dequeue the highest-priority SC |

#### `Scheduler::Release` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `queue` | `Queue<Sc>` | Release queue |
| `lock` | `Spinlock` | Lock for cross-CPU release |
| `enqueue(Sc*)` | `void` | Enqueue an SC for release |
| `dequeue()` | `auto` | Dequeue an SC |

---

## 5. IPC Primitives

### `Pt` — Portal
**File**: [inc/pt.hpp](../inc/pt.hpp)

IPC entry point. Inherits from `Kobject`.

| Member | Kind | Description |
|---|---|---|
| `ec` | `Refptr<Ec> const` (private) | Bound EC (handler) |
| `ip` | `uintptr_t const` (private) | Entry instruction pointer |
| `id` | `Atomic<uintptr_t>` (private) | Caller-visible identifier |
| `mtd` | `Atomic<Mtd_arch>` (private) | Message transfer descriptor |
| `create(Status&, Ec*, uintptr_t)` | `[[nodiscard]] static Pt*` | Factory |
| `destroy()` | `void override final` | Destructor |
| `get_ec()` | `Ec*` | Returns bound EC |
| `get_ip()` | `uintptr_t` | Returns entry IP |
| `get_id()` | `uintptr_t` | Returns ID |
| `get_mtd()` | `Mtd_arch` | Returns MTD |
| `set_id(uintptr_t)` | `void` | Sets ID |
| `set_mtd(Mtd_arch)` | `void` | Sets MTD |
| `collect()` | `void override final` (private) | RCU collect callback |

---

### `Sm` — Semaphore
**File**: [inc/sm.hpp](../inc/sm.hpp)

Counting semaphore. Inherits from `Kobject` and (privately) `Queue<Ec>`.

| Member | Kind | Description |
|---|---|---|
| `pd` | `Refptr<Pd> const` (private) | Owner PD |
| `cnt` | `uint64_t` (private) | Counter value |
| `ptr` | `void* const` (private) | Associated pointer (for interrupt SMs) |
| `iid` | `iid_t const` (private) | Interrupt identifier |
| `lock` | `Spinlock` (private) | Lock |
| `create(Status&, Pd*, uintptr_t, void*)` | `[[nodiscard]] static Sm*` | Factory |
| `destroy()` | `void override final` | Destructor |
| `get_ptr()` | `auto` | Returns associated pointer (asserts `SM_INT`) |
| `get_iid()` | `auto` | Returns interrupt ID (asserts `SM_INT`) |
| `dn(Ec*, bool, uint64_t)` | `[[nodiscard]] Status` | Down (wait) operation |
| `up()` | `Status` | Up (signal) operation |
| `timeout(Ec*)` | `void` | Handle timeout on a blocked EC |
| `collect()` | `void override final` (private) | RCU collect callback |

---

### `Dc` — Device Context
**File**: [inc/dc.hpp](../inc/dc.hpp)

Represents a device assignment. Inherits from `Kobject` and `Dc_state`.

| Member | Kind | Description |
|---|---|---|
| `pd` | `Refptr<Pd> const` (private) | Owner PD |
| `Dc(Refptr<Pd>&, uint64_t, uint64_t, uint64_t)` | constructor | Initialises device context state |
| `create(Status&, Pd*, uint64_t, uint64_t, uint64_t)` | `[[nodiscard]] static Dc*` | Factory |
| `destroy()` | `void override final` | Destructor |
| `collect()` | `void override final` (private) | RCU collect callback |

---

### `Dc_state` — Device Context State (RISC-V)
**File**: [inc/riscv64/dc_state.hpp](../inc/riscv64/dc_state.hpp)

| Member | Kind | Description |
|---|---|---|
| `did` | `uint32_t const` | Device ID |
| `sid` | `uint32_t const` | Stream ID |
| `smmu` | `Smmu* const` | IOMMU pointer (always `nullptr` on RISC-V) |

---

## 6. Memory Management

### `Buddy` — Buddy Allocator
**File**: [inc/buddy.hpp](../inc/buddy.hpp)

Physical page frame allocator.

| Member | Kind | Description |
|---|---|---|
| `max_order` | `static constexpr order_t` | Maximum allocation order |
| `lock` | `static Spinlock` | Global allocator lock |
| `min_idx` / `max_idx` | `static index_t` | Block index range |
| `blk_base` | `static Block*` | Base of block metadata array |
| `freelist` | `static Freelist` | Free block list |
| `waitlist` | `static Waitlist` (CPULOCAL) | Per-CPU wait list |
| `init()` | `static void` | Initialise the allocator |
| `alloc(order_t, Fill)` | `[[nodiscard]] static void*` | Allocate pages |
| `free(void*)` | `static void` | Free pages |
| `wait(void*)` | `static void` | Free pages to wait list (deferred coalescing) |
| `free_wait()` | `static void` | Process the wait list |
| `coalesce(Block*)` | `static void` (private) | Coalesce buddy blocks |

#### `Buddy::Fill` (enum class)
`NONE`, `BITS0`, `BITS1`

#### `Buddy::Block` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `ord` | `order_t` | Block order |
| `tag` | `Tag` | `USED` or `FREE` |

#### `Buddy::Freelist` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `list[max_order+1]` | `Queue<Block>` | Per-order free lists |
| `enqueue(Block*)` / `dequeue(Block*)` / `dequeue(order_t)` | void/auto | List operations |

#### `Buddy::Waitlist` (nested class, private)
| Member | Kind | Description |
|---|---|---|
| `list` | `Queue<Block>` | Deferred-free list |
| `enqueue(Block*)` / `dequeue()` | void/auto | List operations |

---

### `Slab_cache` — Slab Allocator
**File**: [inc/slab.hpp](../inc/slab.hpp)

Fixed-size object allocator.

| Member | Kind | Description |
|---|---|---|
| `bsz` | `uint16_t const` (private) | Buffer (object) size |
| `bps` | `uint16_t const` (private) | Buffers per slab |
| `curr` | `Slab*` (private) | Current (partial) slab |
| `head` | `Slab*` (private) | Head of slab list |
| `lock` | `Spinlock` (private) | Allocator spinlock |
| `Slab_cache(size_t, size_t)` | constructor | Construct with object size and alignment |
| `alloc()` | `[[nodiscard]] void*` | Allocate one object |
| `free(void*)` | `void` | Free one object |

---

### `Kmem` — Kernel Memory Management
**File**: [inc/kmem.hpp](../inc/kmem.hpp)

Static utility class for kernel virtual ↔ physical address translation.

| Member | Kind | Description |
|---|---|---|
| `offset` | `static uintptr_t` (private) | Runtime virtual-to-physical offset |
| `phys_to_virt(uintptr_t)` / `virt_to_phys(uintptr_t)` | static (private) | Runtime conversions |
| `sym_to_virt(void const*)` | `static auto` | Link symbol → virtual |
| `sym_to_phys(void const*)` | `static auto` | Link symbol → physical |
| `ptr_to_phys(void const*)` | `static auto` | Virtual pointer → physical |
| `phys_to_ptr(uintptr_t)` | `static auto` | Physical → virtual pointer |
| `loc_to_glb(cpu_t, T*)` | `static auto` | Convert CPULOCAL pointer to global alias |

---

## 7. Page Tables

### `Ptab<T, I, O>` — Page Table (Template)
**File**: [inc/ptab.hpp](../inc/ptab.hpp)

Generic multi-level page table implementation. `T` is the PTE type, `I` is the input (virtual) address type, `O` is the output (physical) address type.

| Member | Kind | Description |
|---|---|---|
| `entry` | `PTE` (protected) | The root PTE (atomic) |
| `mll` | `static unsigned` (private) | Maximum leaf level |
| `lookup(IAddr, OAddr&, unsigned&, Memattr&)` | `Permissions` | Look up a virtual address |
| `update(IAddr, OAddr, unsigned, Permissions, Memattr)` | `Status` | Update a mapping |
| `root_init(unsigned)` | `[[nodiscard]] auto` | Initialise root page table |
| `root_addr()` | `auto` | Get root physical address |
| `set_mll(unsigned)` | `static void` | Cap the maximum leaf level |
| `walk(IAddr, unsigned, bool)` | `[[nodiscard]] PTE*` (protected) | Walk the page table |
| `diverge(IAddr, IAddr)` | `static constexpr unsigned` (protected) | Level at which two addresses diverge |
| `init(unsigned, OAddr, OAddr)` | `void` (private) | Initialise entries |
| `deallocate(unsigned)` | `void` (private) | Recursively deallocate |

#### `Ptab::Entry` (nested class)
| Member | Kind | Description |
|---|---|---|
| `val` | `OAddr` (protected) | Raw PTE value |
| `bpl` | `static constexpr unsigned` | Bits per level |
| `lev(unsigned)` | `static constexpr auto` | Compute number of levels |
| `lev_bit(unsigned)` / `lev_ent(unsigned)` / `lev_ord(unsigned)` / `lev_idx(unsigned, IAddr)` | static constexpr | Level geometry helpers |
| `addr_mask()` | `static constexpr auto` | Address mask |
| `page_size(unsigned)` / `offs_mask(unsigned)` | static constexpr | Page size and offset mask |
| `addr(unsigned)` | `auto` | Extract the physical address from the PTE |
| `operator->()` | `Ptab*` | Follow PTE as table pointer |
| `operator==(Entry const&)` | `bool` | Equality comparison |

---

### `Pte<T, I, O>` — Page Table Entry (RISC-V)
**File**: [inc/riscv64/ptab_pte.hpp](../inc/riscv64/ptab_pte.hpp)

RISC-V specific PTE logic. Inherits from `Ptab<T,I,O>::Entry`.

| Member | Kind | Description |
|---|---|---|
| `type(unsigned)` | `auto` | Classifies PTE as `HOLE`, `LEAF`, or `PTAB` |
| `publish()` | `static void` | Issues `sfence.vma` barrier |
| `pas(unsigned)` | `static constexpr auto` | Physical address size (56 bits for Sv39) |

---

### `Hpt` — RISC-V Sv39 Page Table Entry
**File**: [inc/riscv64/ptab_hpt.hpp](../inc/riscv64/ptab_hpt.hpp)

Sv39-specific page table entry. Inherits from `Pte<Hpt, uint64_t, uint64_t>`.

| Member | Kind | Description |
|---|---|---|
| `ibits` | `static constexpr unsigned` | 39 (Sv39 virtual address width) |
| `ptab_attr` | `static constexpr auto` | Non-leaf PTE attribute (`ATTR_P`) |
| `page_attr(unsigned, Permissions, Memattr)` | `static OAddr` | Compute leaf PTE attributes |
| `page_pm()` | `auto` | Extract permissions from a PTE |
| `page_ma(unsigned)` | `auto` | Extract memory attributes (always `ram()`) |
| `invalidate()` / `invalidate(uintptr_t)` / `invalidate(uintptr_t, unsigned)` | `static void` | TLB invalidation |

PTE attribute bits: `ATTR_P` (Valid), `ATTR_R` (Read), `ATTR_W` (Write), `ATTR_X` (Execute), `ATTR_U` (User), `ATTR_G` (Global), `ATTR_A` (Accessed), `ATTR_D` (Dirty).

---

### `Hptp` — RISC-V Page Table Pointer
**File**: [inc/riscv64/ptab_hpt.hpp](../inc/riscv64/ptab_hpt.hpp)

Wraps `Ptab<Hpt, uint64_t, uint64_t>` with RISC-V specific operations.

| Member | Kind | Description |
|---|---|---|
| `master` | `static Hptp` (private) | Kernel master page table |
| `page_size(unsigned)` / `offs_mask(unsigned)` | `static auto` | Delegated to `Hpt` |
| `current()` | `static Hptp` | Read current SATP |
| `make_current()` | `void` | Write SATP and flush TLB |
| `master_map(IAddr, OAddr, unsigned, Permissions, Memattr)` | `static auto` | Map in the master page table |
| `map(IAddr, OAddr, Permissions)` | `static void*` | Map a single page and invalidate |
| `map_tmp(OAddr, size_t, Permissions, Memattr, unsigned)` | `static void*` | Temporary mapping |
| `invalidate()` | `void` | Full TLB flush (`sfence.vma`) |

---

## 8. Synchronisation Primitives

### `Atomic<T>` — Atomic Variable
**File**: [inc/atomic.hpp](../inc/atomic.hpp)

Template providing atomic operations via GCC `__atomic` builtins. Two specialisations:

#### Integral Specialisation (`std::is_integral<T>`)
| Member | Kind | Description |
|---|---|---|
| `val` | `T` (private) | Stored value |
| `load(int)` / `store(T, int)` | `T` / `void` | Atomic load/store with configurable memory ordering |
| `operator T()` / `operator=(T)` | conversions | Load/store via operators |
| `operator++` / `--` / `+=` / `-=` / `^=` / `|=` / `&=` | `T` | Atomic arithmetic (ACQ_REL) |
| `operator++(int)` / `operator--(int)` (postfix) | `T` | Fetch-then-modify |
| `fetch_add/sub/xor/or/and(T)` | `T` | Fetch-and-modify operations |
| `exchange(T&, T&)` / `exchange_n(T)` | void/T | Atomic exchange |
| `compare_exchange(T&, T&)` / `compare_exchange_n(T&, T)` | `bool` | Compare-and-swap |
| `test_and_set(T)` / `test_and_clr(T)` | `T` | Test-and-set/clear individual bits |

#### Non-Integral Specialisation (pointers, etc.)
| Member | Kind | Description |
|---|---|---|
| `load(int)` / `store(T, int)` | `T` / `void` | Atomic load/store |
| `operator T()` / `operator->()` / `operator=(T)` | conversions |
| `exchange(T&, T&)` | `void` | Atomic exchange |
| `compare_exchange(T&, T&)` | `bool` | Compare-and-swap |

---

### `Spinlock` — Ticket Spinlock (RISC-V)
**File**: [inc/riscv64/spinlock.hpp](../inc/riscv64/spinlock.hpp)

| Member | Kind | Description |
|---|---|---|
| `val` | `uint32_t` (private) | `nxt[31:16] cur[15:0]` packed ticket |
| `lock()` | `void` | Acquire with LR/SC and spinning |
| `unlock()` | `void` | Release by incrementing `cur` |

---

### `Lock_guard<T>` — RAII Lock Guard
**File**: [inc/lock_guard.hpp](../inc/lock_guard.hpp)

| Member | Kind | Description |
|---|---|---|
| `lock` | `T&` (private) | Reference to managed lock |
| `Lock_guard(T&)` | constructor | Acquires the lock |
| `~Lock_guard()` | destructor | Releases the lock |

---

## 9. Timeout Infrastructure

### `Timeout` — Timeout Base
**File**: [inc/timeout.hpp](../inc/timeout.hpp)

Sorted linked list of pending timeouts. Per-CPU.

| Member | Kind | Description |
|---|---|---|
| `time` | `uint64_t` (private) | Expiry time |
| `prev` / `next` | `Timeout*` (private) | Doubly-linked list pointers |
| `list` | `static Timeout*` (CPULOCAL) | Per-CPU timeout list head |
| `trigger()` | pure virtual (private) | Called when timeout fires |
| `enqueue(uint64_t)` | `void` | Insert into timeout list |
| `dequeue()` | `uint64_t` | Remove from timeout list |
| `check()` | `static void` | Process expired timeouts |
| `sync()` | `static void` | Synchronise timer with next timeout |
| `idle()` | `static uint64_t` | Time until next timeout |

---

### `Timeout_budget` — Budget Timeout
**File**: [inc/timeout_budget.hpp](../inc/timeout_budget.hpp)

Fires when an SC's time budget expires. Inherits from `Timeout`.

| Member | Kind | Description |
|---|---|---|
| `trigger()` | `void override` (private) | Budget expiration handler |
| `timeout` | `static Timeout_budget` (CPULOCAL) | Per-CPU instance |

---

### `Timeout_hypercall` — Hypercall Timeout
**File**: [inc/timeout_hypercall.hpp](../inc/timeout_hypercall.hpp)

Fires when a timed semaphore `dn()` operation times out. Inherits from `Timeout`.

| Member | Kind | Description |
|---|---|---|
| `sm` | `Sm*` (private) | Associated semaphore |
| `trigger()` | `void override final` (private) | Timeout handler |
| `enqueue(uint64_t, Sm*)` | `void` | Enqueue with semaphore association |

---

## 10. Console & I/O

### `Console` — Generic Console
**File**: [inc/console.hpp](../inc/console.hpp)

Abstract console interface using a linked list of active console backends.

| Member | Kind | Description |
|---|---|---|
| `dormant` / `enabled` | `static Console*` | Lists of dormant/enabled consoles |
| `lock` | `static Spinlock` | Console lock |
| `outc(char)` | `[[nodiscard]] virtual bool` (private, pure) | Output a single character |
| `putc(char)` | `static void` (private) | Output to all enabled consoles |
| `print_num(...)` / `print_str(...)` | static (private) | Number/string formatting |
| `vprintf(char const*, va_list)` | static (private) | Formatted output |
| `init()` / `fini()` | `[[nodiscard]] virtual bool` (protected) | Initialise/finalise backend |
| `match_dbgp(Type, Subtype)` | `[[nodiscard]] virtual bool` (protected) | Match debug port |
| `enable()` / `disable()` | protected | Move between dormant/enabled lists |
| `print(char const*, ...)` | `static void` | Public formatted print |
| `flush()` | `static void` | Flush all enabled consoles |

Inherits from `List<Console>`.

---

### `Console_mbuf_mmio` — Memory-Buffer Console MMIO Region
**File**: [inc/console_mbuf.hpp](../inc/console_mbuf.hpp)

| Member | Kind | Description |
|---|---|---|
| `r_idx` | `Atomic<uint32_t>` | Read index |
| `w_idx` | `Atomic<uint32_t>` | Write index |
| `buffer[entries]` | `Atomic<char>` | Circular character buffer |
| `ord` / `size` / `entries` | `static constexpr` | Buffer sizing constants |
| `operator new(size_t)` | `[[nodiscard]] static void*` | Allocates via `Buddy::alloc` |

---

### `Console_mbuf` — Memory-Buffer Console
**File**: [inc/console_mbuf.hpp](../inc/console_mbuf.hpp)

Console backend that writes to a shared memory buffer. Privately inherits from `Console`.

| Member | Kind | Description |
|---|---|---|
| `sm` | `Sm*` (private) | Notification semaphore |
| `regs` | `Console_mbuf_mmio* const` (public) | Pointer to MMIO region |
| `singleton` | `static Console_mbuf` | Singleton instance |
| `addr()` | `static auto` | Physical address of the mbuf |
| `size()` | `static auto` | Size of the mbuf |
| `outc(char)` | `[[nodiscard]] bool override final` (private) | Write a character to buffer |

---

### `Console_uart` — UART Console Base
**File**: [inc/console_uart.hpp](../inc/console_uart.hpp)

Abstract UART console. Protectedly inherits from `Console`.

| Member | Kind | Description |
|---|---|---|
| `clock` | `unsigned const` (protected) | UART clock rate |
| `mmap` | `uintptr_t` (protected) | MMIO mapping address |
| `regs` | `Regs` (protected) | Register addresses |
| `tx(uint8_t)` | virtual (private, pure) | Transmit one byte |
| `tx_busy()` | `[[nodiscard]] virtual bool` (private, pure) | Check if transmitter is busy |
| `tx_full()` | `[[nodiscard]] virtual bool` (private, pure) | Check if TX FIFO is full |
| `fini()` | `[[nodiscard]] bool override final` (private) | Wait for transmit completion |
| `outc(char)` | `[[nodiscard]] bool override final` (private) | Output a character with timeout |
| `sync()` | `void` (private) | Sync from memory buffer console |
| `setup(Regs)` | `bool` (protected) | Set up UART from register config |

#### `Console_uart::Regs` (nested struct)
| Member | Type | Description |
|---|---|---|
| `mem` | `uint64_t` | MMIO physical address |
| `pio` | `uint16_t` | Port I/O address |
| `shl` | `uint8_t` | Register shift |

---

### `Console_sbi` — SBI Console (RISC-V)
**File**: [inc/riscv64/console_sbi.hpp](../inc/riscv64/console_sbi.hpp)

Console backed by SBI `putc` calls. Privately inherits from `Console`.

| Member | Kind | Description |
|---|---|---|
| `sbi[]` | `static Console_sbi` | Static instances |
| `outc(char)` | `bool` override final | Outputs via `Sbi::putc()` |

---

## 11. Data Structures

### `Queue<T>` — Intrusive Circular Queue
**File**: [inc/queue.hpp](../inc/queue.hpp)

| Member | Kind | Description |
|---|---|---|
| `head` | `Element*` (private) | Head of circular list |
| `empty()` | `bool` | Check if empty |
| `enqueue(T*, bool)` | `bool` | Enqueue as head or tail; returns true if queue was empty |
| `enqueue_head(T*)` / `enqueue_tail(T*)` | `auto` | Convenience wrappers |
| `dequeue(T*)` | `void` | Remove a specific element |
| `dequeue_head()` | `auto` | Remove and return the head |

#### `Queue<T>::Element` (nested class)
| Member | Kind | Description |
|---|---|---|
| `prev` / `next` | `Element*` (private) | Doubly-linked list pointers |
| `get_prev()` / `get_next()` | `auto` | Accessors |
| `set_prev(Element*)` / `set_next(Element*)` | `void` | Mutators |

---

### `List<T>` — Singly-Linked List
**File**: [inc/list.hpp](../inc/list.hpp)

| Member | Kind | Description |
|---|---|---|
| `next` | `T*` (protected) | Next element |
| `List(T*&)` | constructor | Inserts self into a list |
| `insert(T*&)` | protected | Insert into a list |
| `remove(T*&)` | protected | Remove from a list |

---

### `Bitmap<B, I>` — Atomic Bitmap
**File**: [inc/bitmap.hpp](../inc/bitmap.hpp)

Fixed-size atomic bitmap of `B` bits, optionally initialised to all-ones (`I=true`).

| Member | Kind | Description |
|---|---|---|
| `bitmap[]` | `Atomic<uintptr_t>` array (private) | Storage |
| `clr(size_t)` | `void` | Clear a bit |
| `set(size_t)` | `void` | Set a bit |
| `tst(size_t)` | `bool` | Test a bit |
| `cfg(size_t, bool)` | `void` | Set or clear based on boolean |

---

## 12. Hardware Abstraction

### `Cpu` — Central Processing Unit (RISC-V)
**File**: [inc/riscv64/cpu.hpp](../inc/riscv64/cpu.hpp)

| Member | Kind | Description |
|---|---|---|
| `ptab` | `static uint64_t` (CPULOCAL, private) | HS-mode page table root |
| `hartid` | `static uint64_t` (CPULOCAL, private) | Hart ID |
| `mvendorid/marchid/mimpid/misa` | `static uint64_t` (CPULOCAL, private) | Machine identification CSRs |
| `boot_lock` | `static Spinlock` (private) | Boot serialisation lock |
| `id` | `static cpu_t` (CPULOCAL, public) | Logical CPU ID |
| `hazard` | `static unsigned` (CPULOCAL, public) | Pending hazard flags |
| `count` | `static cpu_t` (public) | Number of online CPUs |
| `boot_cpu` | `static cpu_t` (public) | Bootstrap CPU ID |
| `online` | `static Atomic<uint32_t>` (public) | Online CPU bitmask |
| `feature_h` | `static bool` (public) | H-extension presence |
| `remote_ptab(cpu_t)` / `remote_hartid(cpu_t)` | `static uint64_t` | Read from another CPU's data |
| `init(cpu_t, unsigned)` / `init_bsp()` / `init_ap()` | `static void` | CPU initialisation |
| `hpt_base()` | `static auto` | Returns page table root |
| `affinity()` | `static unsigned` | Returns hart ID |
| `halt()` | `static void` | Execute WFI |
| `preempt_disable()` / `preempt_enable()` | `static void` | Toggle SIE in sstatus |
| `preemption_point()` | `static void` | Brief preemption window |
| `fini()` | `[[noreturn]] static void` | Halt loop |
| `reboot_or_shutdown()` | `[[noreturn]] static void` | System reset |

---

### `Timer` — Timer (RISC-V)
**File**: [inc/riscv64/timer.hpp](../inc/riscv64/timer.hpp)

| Member | Kind | Description |
|---|---|---|
| `freq` | `static uint64_t` (private) | Timer frequency |
| `time()` | `static uint64_t` | Read `time` CSR |
| `frequency()` | `static uint64_t` | Get timer frequency |
| `set_dln(uint64_t)` | `static void` | Set timer compare (`stimecmp`) |
| `stop()` | `static void` | Disable timer interrupt (clear STIE) |
| `init()` | `static void` | Initialise timer |

---

### `Stc` — System Time Counter
**File**: [inc/stc.hpp](../inc/stc.hpp)

| Member | Kind | Description |
|---|---|---|
| `freq` | `static uint64_t` | STC frequency in Hz (default 1 GHz) |
| `ms_to_ticks(uint32_t)` | `static auto` | Convert ms to ticks |
| `ticks_to_ms(uint64_t)` | `static auto` | Convert ticks to ms |
| `ticks_to_us(uint64_t)` | `static auto` | Convert ticks to µs |

---

### `Counter` — Event Counter (RISC-V)
**File**: [inc/riscv64/counter.hpp](../inc/riscv64/counter.hpp)

| Member | Kind | Description |
|---|---|---|
| `val` | `Atomic<unsigned>` (private) | Counter value |
| `req[NUM_IPI]` / `loc[NUM_LOC]` | `static Counter` (CPULOCAL) | IPI/local interrupt counters |
| `schedule` / `helping` | `static Counter` (CPULOCAL) | Schedule/help counters |
| `inc()` | `void` | Increment |
| `get(cpu_t)` | `unsigned` | Read from another CPU |

---

### `Hazard` — Hazard Flags
**File**: [inc/hazard.hpp](../inc/hazard.hpp)

| Member | Kind | Description |
|---|---|---|
| `val` | `Atomic<hazard_t>` (private) | Flags |
| `Hazard(hazard_t)` | constructor | Initialise |
| `operator hazard_t()` | conversion | Read all flags |
| `set(hazard_t)` / `clr(hazard_t)` / `tas(hazard_t)` | void/auto | Atomic set, clear, test-and-set |

Hazard bits: `SCHED`, `SLEEP`, `RCU`, `TR` (x86), `FPU`, `TSC` (x86), `RECALL`, `ILLEGAL`.

---

### `Interrupt` — Interrupt Handling (RISC-V)
**File**: [inc/riscv64/interrupt.hpp](../inc/riscv64/interrupt.hpp)

| Member | Kind | Description |
|---|---|---|
| `Request` | enum | `RRQ` (reschedule), `RKE` (kernel entry) |
| `init()` | `static void` | Initialise interrupt controller |
| `handle_irq()` / `handle_ipi()` / `handle_timer()` | `static void` | Interrupt dispatch |
| `send_ipi(unsigned, unsigned)` / `send_cpu(Request, cpu_t)` / `send_exc(Request)` | `static void` | IPI sending |
| `conf(unsigned, bool)` | `static void` | Configure interrupt |
| `deactivate(unsigned)` / `deactivate(Sm*)` | `static void` | Deactivate interrupt |
| `get_ptr(iid_t)` | `static Refptr<Sm>*` | Always `nullptr` (stub) |
| `assign(...)` | `static Status` | Always `BAD_FTR` (stub) |

---

### `Intid` — Interrupt Identifiers (RISC-V)
**File**: [inc/riscv64/intid.hpp](../inc/riscv64/intid.hpp)

| Member | Kind | Description |
|---|---|---|
| `NUM_IPI` / `NUM_LOC` / `NUM_EXT` / `NUM_INT` | `static constexpr unsigned` | Interrupt count constants |
| `Type` | enum class | `IPI`, `TIMER`, `EXT`, `UNKNOWN` |
| `CAUSE_SSI/STI/SEI` | `static constexpr unsigned` | RISC-V interrupt cause values |
| `type(unsigned)` | `static constexpr auto` | Map cause to type |

---

### `Clint` — Core Local Interruptor (RISC-V)
**File**: [inc/riscv64/clint.hpp](../inc/riscv64/clint.hpp)

| Member | Kind | Description |
|---|---|---|
| `clint_base` | `static uintptr_t` (private) | MMIO base address |
| `read32/64(unsigned)` / `write32/64(unsigned, T)` | static (private) | Register access |
| `init()` | `static void` | Initialise CLINT |
| `time()` | `static uint64_t` | Read MTIME |
| `set_timecmp(unsigned, uint64_t)` | `static void` | Set timer compare for a hart |
| `send_ipi(unsigned)` | `static void` | Send IPI to a hart |
| `clear_ipi(unsigned)` | `static void` | Clear IPI for a hart |

---

### `Plic` — Platform-Level Interrupt Controller (RISC-V)
**File**: [inc/riscv64/plic.hpp](../inc/riscv64/plic.hpp)

| Member | Kind | Description |
|---|---|---|
| `plic_base` | `static uintptr_t` (private) | MMIO base address |
| `read(unsigned)` / `write(unsigned, uint32_t)` | static (private) | Register access |
| `init(cpu_t)` | `static void` | Initialise PLIC for a CPU context |
| `conf(unsigned, bool)` | `static void` | Configure an interrupt |
| `send_sgi(unsigned, cpu_t)` | `static void` | Send software-generated interrupt |
| `set_priority(unsigned, unsigned)` | `static void` | Set interrupt priority |
| `enable(unsigned, unsigned)` / `disable(unsigned, unsigned)` | `static void` | Enable/disable interrupt for context |
| `set_threshold(unsigned, unsigned)` | `static void` | Set priority threshold |
| `claim(unsigned)` | `static unsigned` | Claim an interrupt |
| `complete(unsigned, unsigned)` | `static void` | Complete an interrupt |

---

### `Sbi` — Supervisor Binary Interface (RISC-V)
**File**: [inc/riscv64/sbi.hpp](../inc/riscv64/sbi.hpp)

| Member | Kind | Description |
|---|---|---|
| `EXT_BASE/TIMER/CONSOLE/IPI/RFENCE/HSM/SRST/PMU` | `static constexpr unsigned long` | Extension IDs |
| `TIMER_SET`, `CONSOLE_WRITE/READ/WRITE_BYTE`, `IPI_SEND`, etc. | `static constexpr unsigned long` | Function IDs |
| `SUCCESS`, `ERR_FAILED`, `ERR_NOT_SUPPORTED`, etc. | `static constexpr long` | Error codes |
| `call(ext, fid, a0..a5)` | `static Result` | Generic SBI ecall wrapper |
| `set_timer(uint64_t)` | `static void` | Set timer compare |
| `puts/gets/putc` | static | Console I/O |
| `send_ipi(mask, base)` | `static void` | Send IPI |
| `remote_fence_i/sfence_vma(...)` | `static void` | Remote fence operations |
| `hart_start/stop/status(...)` | `static long` | Hart lifecycle management |
| `system_reset(type, reason)` | `static void` | System reset |

#### `Sbi::Result` (nested struct)
| Member | Type | Description |
|---|---|---|
| `error` | `long` | Error code |
| `value` | `long` | Return value |

---

### `Smmu` — System MMU (RISC-V stub)
**File**: [inc/riscv64/smmu.hpp](../inc/riscv64/smmu.hpp)

All methods return failure/null (no IOMMU support).

| Member | Kind | Description |
|---|---|---|
| `avail_smg()` / `avail_ctx()` | `static uint8_t` | Both return 0 |
| `initialize()` | `[[nodiscard]] static bool` | Returns true |
| `lookup(uint64_t)` | `static Smmu*` | Returns `nullptr` |
| `using_iid(unsigned)` | `static bool` | Returns `false` |
| `interrupt(unsigned)` | `static void` | No-op |
| `tlb_invalidate_all(Sdid)` | `static void` | No-op |
| `assign_dev(Dc const*, Space_dma*)` | `Status` | Returns `BAD_FTR` |

---

### `Pci` — PCI Configuration Space
**File**: [inc/pci.hpp](../inc/pci.hpp)

Manages PCI device enumeration and configuration access.

| Member | Kind | Description |
|---|---|---|
| `cfg_size` / `seg_shft/bus_shft/dev_shft` | `static constexpr` | Layout constants |
| `seg_grps` | `static constexpr auto` | Number of mappable segment groups |
| `ecam_addr(pci_t, unsigned)` | `static constexpr uintptr_t` | ECAM address calculation |
| `pci(seg, bus, dev, fun)` | `static constexpr pci_t` | Construct PCI address |
| `seg/bdf/bus/ari/dev/fun(pci_t)` | `static constexpr` | Decompose PCI address |
| `init_bus(...)` / `init_seg(...)` | `static` | Bus/segment initialisation |

#### `Pci::Cfg` (nested struct)
Contains register offset enums: `Reg32`, `Reg16`, `Reg8` for PCI configuration header fields.

#### `Pci::Device` (nested class)
Inherits from `List<Device>`, `Cap_pmi`, `Cap_pcix`, `Cap_pcie`, `Cap_sriov`.

| Member | Kind | Description |
|---|---|---|
| `pci` | `pci_t const` (private) | PCI address |
| `lev` | `uint8_t const` (private) | Bus topology level |
| `smmu` | `Smmu*` (private) | Associated IOMMU |
| `cache` | `static Slab_cache` (private) | Device slab cache |
| `list` | `static Device*` (private) | Device list head |
| `Device(pci_t, uint8_t)` | constructor |
| `read(Reg8/16/32)` | `auto` | Configuration read |
| `write(Reg8/16/32, T)` | `void` | Configuration write |
| `cap<T>()` | `auto` | Get capability pointer for a type |
| `claim_all(Smmu*)` / `claim_dev(Smmu*, pci_t)` | `static void/bool` | IOMMU device claiming |
| `find_smmu(pci_t)` | `static Smmu*` | Find IOMMU for a device |
| `operator new/delete` | slab-based |

PCI capability structs: `Pcap`, `Ecap`, `Cap_pmi`, `Cap_pcix`, `Cap_pcie`, `Cap_sriov` — each with register offset enums.

---

### `Mmio` — Memory-Mapped I/O
**File**: [inc/mmio.hpp](../inc/mmio.hpp)

| Member | Kind | Description |
|---|---|---|
| `mmio_base` | `static Atomic<uintptr_t>` (private) | Next MMIO allocation address |
| `phys` | `uintptr_t const` (protected) | Physical base |
| `mmio` | `uintptr_t const` (protected) | MMIO virtual base |
| `mmio_size` | `size_t const` (protected) | MMIO region size |
| `alloc_mmio(phys, size, a, r)` | `static uintptr_t` (protected) | Allocate and map an MMIO region |
| `Mmio(uintptr_t, size_t, Memattr, bool)` | constructor | Allocate and map |

---

### `Fpu` — Floating Point Unit (RISC-V)
**File**: [inc/riscv64/fpu.hpp](../inc/riscv64/fpu.hpp)

| Member | Kind | Description |
|---|---|---|
| `regs.f[32]` | `uint64_t` (private) | 32 double-precision FP registers |
| `regs.fcsr` | `uint64_t` (private) | FP control/status register |
| `size` / `alignment` | `static constexpr size_t` | Context size (264 bytes) / alignment (8) |
| `load()` | `void` | Load FPU context from memory |
| `save()` | `void` | Save FPU context to memory |
| `create(Slab_cache&, Hazard&)` | `[[nodiscard]] static Fpu*` | Factory |
| `destroy(Slab_cache&)` | `void` | Destroy via slab |
| `enable()` / `disable()` | `static void` | Enable/disable FPU in `sstatus.FS` |
| `operator new/delete` | slab-based |

---

## 13. Utility & Support Classes

### `Hip` — Hypervisor Information Page
**File**: [inc/hip.hpp](../inc/hip.hpp)

Shared page between hypervisor and userland containing system configuration.

| Member | Kind | Description |
|---|---|---|
| `signature` / `checksum` / `length` | private | Header fields |
| `nova_p/e_addr` / `mbuf_p/e_addr` / `root_p/e_addr` | private | Address ranges |
| `tmr_frq` | private | Timer frequency |
| `sbw_obj..msr` / `mco_obj..msr` | private | Space bit widths / max capability orders |
| `cpu_bsp/max` / `kid_max` | private | CPU topology |
| `sel_hst/gst_arch/nova` | private | Event selectors |
| `features` | `Atomic<feat_t>` (private) | Feature flags |
| `arch` | `Hip_arch` (private) | Architecture-specific data |
| `hip` | `static Hip*` | Pointer to the HIP |
| `feature(Feature)` / `set_feature(Feature)` / `clr_feature(Feature)` | `static` | Feature flag operations |
| `build(uint64_t, uint64_t)` | `void` | Populate the HIP |

---

### `Hip_arch` — Architecture-Specific HIP (RISC-V)
**File**: [inc/riscv64/hip_arch.hpp](../inc/riscv64/hip_arch.hpp)

| Member | Kind | Description |
|---|---|---|
| `num_plic_ctx` / `num_plic_irq` | `uint32_t` (private) | PLIC parameters |
| `reserved` | `uint64_t` (private) | Reserved |
| `Feature` | enum class | `SMMU`, `PLIC`, `CLINT`, `IOMMU` |
| `build()` | `void` | Populate arch-specific HIP |

---

### `Memattr` — Memory Attributes (RISC-V)
**File**: [inc/riscv64/memattr.hpp](../inc/riscv64/memattr.hpp)

| Member | Kind | Description |
|---|---|---|
| `obits` | `static constexpr unsigned` | Output physical address bits (56) |
| `kimax` | `static constexpr unsigned` | Key ID max (0) |
| `Type` | enum class | `PMA`, `NC`, `IO` |
| `type` | `Type` (private) | Memory type |
| `Memattr()` / `Memattr(Type)` / `Memattr(uint32_t)` | constructors |
| `value()` / `valid()` / `operator==` | `constexpr` | Accessors |
| `ram()` / `dev()` / `gfx()` | `static constexpr Memattr` | Predefined types |

---

### `Paging` — Paging Permissions
**File**: [inc/paging.hpp](../inc/paging.hpp)

| Member | Kind | Description |
|---|---|---|
| `Permissions` | enum | `NONE`, `R`, `W`, `XU`, `XS`, `API`, `U`, `K`, `G`, `SS` |

---

### `Multiboot` — Multiboot Parameters
**File**: [inc/multiboot.hpp](../inc/multiboot.hpp)

| Member | Kind | Description |
|---|---|---|
| `p0/p1/p2` | `static uintptr_t` | Multiboot parameters |
| `t0/t1/t2` | `static uint64_t` | Timing parameters |
| `cl` / `ea` / `ra` | `static uint64_t` | Command line, entry address, RSDP address |

---

### `Cmdline` — Command Line Parser
**File**: [inc/cmdline.hpp](../inc/cmdline.hpp)

| Member | Kind | Description |
|---|---|---|
| `insecure/noccst/nocpst/nodl/nofbuf/nomktme/nopcid/nosmmu/nouart/novpid` | `static bool` | Option flags |
| `options[]` | `static constexpr` | String-to-variable option mapping |
| `init()` | `static void` | Parse the command line |
| `arg_len(char const*&)` | `static size_t` (private) | Get argument length |
| `parse(char const*)` | `static void` (private) | Parse a single option |

---

### `Patch` — Instruction Patching
**File**: [inc/patch.hpp](../inc/patch.hpp)

| Member | Kind | Description |
|---|---|---|
| `off_old/off_new` | `int32_t` (private) | Offsets to old/new instruction |
| `len_old/len_new` | `uint8_t` (private) | Lengths of old/new instruction |
| `tag` | `uint16_t` (private) | Patch tag |
| `applied` | `static uint32_t` | Applied patches bitmask |
| `detect()` / `init()` | `static void` | Detect features and apply patches |

---

### `Signature` — ASCII Signature Utilities
**File**: [inc/signature.hpp](../inc/signature.hpp)

| Member | Kind | Description |
|---|---|---|
| `u32(char const*)` | `static constexpr auto` | Convert 4-char string to little-endian `uint32_t` |
| `u64(char const*)` | `static constexpr auto` | Convert 8-char string to little-endian `uint64_t` |

---

### `Checksum` — Checksum Functions
**File**: [inc/checksum.hpp](../inc/checksum.hpp)

| Member | Kind | Description |
|---|---|---|
| `additive<T>(T const*, unsigned)` | `static auto` | Compute additive checksum |

---

### `Uuid` — Universally Unique Identifier
**File**: [inc/uuid.hpp](../inc/uuid.hpp)

| Member | Kind | Description |
|---|---|---|
| `uuid[2]` | `uint64_t const` (private) | 128-bit UUID packed as two 64-bit values |
| `operator==(Uuid const&)` | `constexpr bool` | Equality |
| `Uuid(uint32_t, uint16_t, uint16_t, uint8_t const(&)[8])` | `constexpr` constructor | Construct from parts |

---

### `Sdid` — SMMU Domain Identifier
**File**: [inc/sdid.hpp](../inc/sdid.hpp)

| Member | Kind | Description |
|---|---|---|
| `val` | `uint8_t const` (private) | Identifier value |
| `allocator` | `static Atomic<uint8_t>` (private) | Monotonic allocator |
| `Sdid()` | constructor | Auto-allocates next ID |
| `operator auto()` | conversion | Returns the ID value |

---

### `Debug` — Debug Port Types
**File**: [inc/debug.hpp](../inc/debug.hpp)

| Member | Kind | Description |
|---|---|---|
| `Type` | enum class : uint16_t | `SERIAL`, `FIREWIRE`, `USB`, `NET` |
| `Subtype` | enum class : uint16_t | Many UART subtypes plus USB subtypes |

---

### `Wait` — Completion Wait
**File**: [inc/wait.hpp](../inc/wait.hpp)

| Member | Kind | Description |
|---|---|---|
| `until(uint32_t ms, auto const& func)` | `static auto` | Spin-wait until `func()` returns true or timeout |

---

### `Cos` — Class of Service (RISC-V)
**File**: [inc/riscv64/cos.hpp](../inc/riscv64/cos.hpp)

Stub — RISC-V has no hardware QoS.

| Member | Kind | Description |
|---|---|---|
| `valid_cos(cos_t)` | `static bool` | Only COS 0 is valid |
| `make_current(cos_t)` | `static void` | No-op |
| `cfg_qos/cfg_l3_mask/cfg_l2_mask/cfg_mb_thrt` | static | All return `BAD_FTR` |

---

### `Event` — Event Selectors (RISC-V)
**File**: [inc/riscv64/event.hpp](../inc/riscv64/event.hpp)

| Member | Kind | Description |
|---|---|---|
| `Selector` | enum | `NONE`, `STARTUP`, `RECALL`, `VTIMER` |
| `hst_arch` / `gst_arch` | `static constexpr auto` | Number of host/guest arch events |
| `hst_max` / `gst_max` | `static constexpr auto` | Max event selectors |

---

### `Integrity` — Integrity Measurement (RISC-V stub)
**File**: [inc/riscv64/integrity.hpp](../inc/riscv64/integrity.hpp)

| Member | Kind | Description |
|---|---|---|
| `root_phys` / `root_size` | `static uint64_t` | Root module address/size |
| `measure()` | `static bool` | Perform integrity measurement |

---

### `Board` — Board Configuration (RISC-V QEMU)
**File**: [inc/riscv64/board_qemu.hpp](../inc/riscv64/board_qemu.hpp)

| Member | Kind | Description |
|---|---|---|
| `PLIC_BASE` / `CLINT_BASE` / `UART_BASE` | `static constexpr unsigned long` | MMIO base addresses |
| `uart[]` | `static constexpr Uart` | UART configuration array |
| `plic` / `clint` | `static constexpr` | PLIC/CLINT base address structs |

#### `Board::Uart` (nested struct)
`type` (Debug::Subtype), `mmio` (unsigned long), `clock` (unsigned)

---

### Endian Types
**File**: [inc/endian.hpp](../inc/endian.hpp)

#### `Aligned<T, B>` — Aligned Endian-Swapping Type
| Member | Kind | Description |
|---|---|---|
| `val` | `T` (private) | Stored value |
| `Aligned(T)` | constructor | Stores with optional byte-swap |
| `operator T()` | conversion | Returns with optional byte-swap |

Type aliases: `Aligned_be<T>`, `Aligned_le<T>`

#### `Unaligned<T, B>` — Unaligned Endian-Swapping Type
| Member | Kind | Description |
|---|---|---|
| `val[sizeof(T)]` | `char` (private) | Byte-array storage |
| `Unaligned(T)` | constructor | Stores via memcpy + byte-swap |
| `operator T()` | conversion | Loads via memcpy + byte-swap |

Type aliases: `Unaligned_be<T>`, `Unaligned_le<T>`

---

### `Sha` / `Hash` — Secure Hash Algorithms
**File**: [inc/hash.hpp](../inc/hash.hpp)

#### `Sha` (base class, protected)
| Member | Kind | Description |
|---|---|---|
| `len` | `size_t` (private) | Total message length |
| `preprocess<T>(T*, uint8_t const*, size_t, bool, void (*)(T*, uint8_t const*))` | protected | Message padding and block processing |
| `sha1(T*, uint8_t const*)` | `static void` (protected) | SHA-1 compression |
| `sha2(T*, uint8_t const*)` | `static void` (protected) | SHA-2 compression |
| Various helper functions | private | `shr`, `rol`, `ror`, `chx`, `maj`, `par`, `sum0/1`, `sig0/1` |
| `cbr[80]` | `static constexpr uint64_t` (private) | Cube root constants |

#### `Hash<F, D, H, T, I...>` (template, inherits `Sha`)
| Member | Kind | Description |
|---|---|---|
| `h[H]` | `T` (private) | Hash state |
| `digest` | `static constexpr auto` | Digest size in bytes |
| `serialize(uint8_t*)` | `void` | Serialise hash to bytes |
| `update(uint8_t const*, size_t, bool)` | `void` | Process message data |

Type aliases: `Hash_sha1_160`, `Hash_sha2_224`, `Hash_sha2_256`, `Hash_sha2_384`, `Hash_sha2_512`

---

## 14. System-Call Interface Structures

**File**: [inc/syscall.hpp](../inc/syscall.hpp)

All structs privately inherit from `Sys_abi` and provide typed accessors for system call parameters.

### `Sys_abi` (RISC-V)
**File**: [inc/riscv64/abi.hpp](../inc/riscv64/abi.hpp)

| Member | Kind | Description |
|---|---|---|
| `s` | `Sys_regs&` (private) | Reference to register file |
| `p0()..p4()` | accessors | Map to `a0`-`a4` (x10-x14) |
| `flags()` | `uint8_t` | Extracts flags from `p0()` |

### Syscall Structures (all in [inc/syscall.hpp](../inc/syscall.hpp))

| Struct | Accessors |
|---|---|
| `Sys_ipc_call` | `timeout()`, `pt()`, `mtd()` |
| `Sys_ipc_reply` | `mtd_a()`, `mtd_u()` |
| `Sys_create_pd` | `op()`, `sel()`, `pd()` |
| `Sys_create_ec` | `flg()`, `sel()`, `pd()`, `hva()`, `evt()`, `cpu()`, `sp()` |
| `Sys_create_sc` | `sel()`, `pd()`, `ec()`, `budget()`, `prio()`, `cos()` |
| `Sys_create_pt` | `sel()`, `pd()`, `ec()`, `ip()` |
| `Sys_create_sm` | `flg()`, `sel()`, `pd()`, `val()` |
| `Sys_create_dc` | `flg()`, `sel()`, `pd()`, `topo()`, `dmar()`, `intr()` |
| `Sys_ctrl_pd` | `src()`, `dst()`, `ssb()`, `dsb()`, `ord()`, `pmm()`, `ma()` |
| `Sys_ctrl_ec` | `strong()`, `ec()` |
| `Sys_ctrl_sc` | `sc()`, `set_time_ticks(uint64_t)` |
| `Sys_ctrl_pt` | `pt()`, `id()`, `mtd()` |
| `Sys_ctrl_sm` | `op()`, `zc()`, `sm()`, `time_ticks()` |
| `Sys_ctrl_hw` | `op()`, `desc()` |
| `Sys_assign_dev` | `dc()`, `dma()` |
| `Sys_assign_int` | `op()`, `sm()`, `dc()`, `cfg()`, `cpu()`, `idx()`, `msi_addr()`, `msi_data()` |

---

## 15. RISC-V Architecture-Specific Classes

### Register File Structures
**File**: [inc/riscv64/regs.hpp](../inc/riscv64/regs.hpp)

#### `Sys_regs`
| Member | Type | Description |
|---|---|---|
| `gpr[31]` | `uintptr_t` | General-purpose registers x1-x31 (x0 is hardwired zero) |

#### `Exc_regs`
| Member | Type | Description |
|---|---|---|
| `sys` | `Sys_regs` | GPR state |
| `csr.sepc` | `uint64_t` | Supervisor exception PC |
| `csr.sstatus` | `uint64_t` | Supervisor status |
| `csr.scause` | `uint64_t` | Supervisor cause |
| `csr.stval` | `uint64_t` | Supervisor trap value |
| `ip()` / `sp()` | references | Aliases for sepc / x2 |
| `mode()` | `unsigned` | Extracts privilege mode from sstatus |
| `cause()` / `is_interrupt()` / `ep()` / `set_ep()` / `set_cause()` | accessors |

#### `Cpu_regs`
| Member | Type | Description |
|---|---|---|
| `exc` | `Exc_regs` | Exception register state |
| `obj` | `Refptr<Space_obj> const` | Object space reference |
| `hst` | `Refptr<Space_hst> const` | Host space reference |
| `hazard` | `Hazard` | Hazard flags |
| `get_obj()` / `get_hst()` | accessors |

---

### `Utcb_arch` — Architecture-Specific UTCB (RISC-V)
**File**: [inc/riscv64/utcb_arch.hpp](../inc/riscv64/utcb_arch.hpp)

| Member | Kind | Description |
|---|---|---|
| `user.gpr[31]` | `uint64_t` (private) | User-mode GPRs (x1-x31) |
| `user.sp` / `user.tp` | `uint64_t` (private) | Stack pointer / thread pointer |
| `csr.sepc/sstatus/scause/stval/satp/sscratch` | `uint64_t` (private) | Supervisor CSRs |
| `sel.gst` | `uint64_t` (private) | Guest space selector |
| `assign_spaces(Cpu_regs&, Space_obj const*)` | `bool` (private) | Assign address spaces |
| `load(Mtd_arch, Cpu_regs const&)` | `void` | Load state from CPU regs into UTCB |
| `save(Mtd_arch, Cpu_regs&, Space_obj const*)` | `bool` | Save state from UTCB into CPU regs |

Size: 320 bytes (0x140).

---

### `Utcb` — User Thread Control Block
**File**: [inc/utcb.hpp](../inc/utcb.hpp)

| Member | Kind | Description |
|---|---|---|
| `mr[Mtd_user::items]` | `uintptr_t` (private, union) | Message registers |
| `state` | `Utcb_arch` (private, union) | Architectural state (overlaps `mr`) |
| `arch()` | `auto` | Returns `&state` |
| `copy(Mtd_user, Utcb*)` | `void` | Copy message registers |
| `operator new/delete` | Buddy-based (aligned to page) |

---

### `Mtd` / `Mtd_user` / `Mtd_arch` — Message Transfer Descriptors
**Files**: [inc/mtd.hpp](../inc/mtd.hpp), [inc/riscv64/mtd_arch.hpp](../inc/riscv64/mtd_arch.hpp)

#### `Mtd` (base)
| Member | Kind | Description |
|---|---|---|
| `mtd` | `uint32_t const` (protected) | Descriptor value |
| `operator auto()` | `uint32_t` | Conversion |

#### `Mtd_user` (user MTD)
| Member | Kind | Description |
|---|---|---|
| `items` | `static constexpr auto` | `PAGE_SIZE(0) / sizeof(uintptr_t)` |
| `count()` | `auto` | Number of message items to transfer |

#### `Mtd_arch` (RISC-V architecture MTD)
Items (bit flags): `POISON`, `ICI`, `GPR`, `FPR`, `EL_SP`, `EL_TP`, `EL_EPC`, `EL_STATUS`, `EL_CAUSE`, `EL_TVAL`, `EL_SATP`, `EL_SCRATCH`, `SPACES`

---

### `Status` — Hypercall Status Codes
**File**: [inc/status.hpp](../inc/status.hpp)

`enum class Status : unsigned` — `SUCCESS`, `TIMEOUT`, `ABORTED`, `OVRFLOW`, `BAD_HYP`, `BAD_CAP`, `BAD_PAR`, `BAD_FTR`, `BAD_CPU`, `BAD_DEV`, `MEM_OBJ`, `MEM_CAP`

---

### ELF Structures
**File**: [inc/elf.hpp](../inc/elf.hpp)

#### `Eh` — ELF Header
| Member | Type | Description |
|---|---|---|
| `ei_magic` | `uint32_t` | Magic number |
| `ei_class` | `Class` | `E32` or `E64` |
| `ei_data` | `Data` | `LSB` or `MSB` |
| `type` | `Type` | `EXEC` |
| `machine` | `Machine` | `X86_32`, `X86_64`, `AARCH64`, `RISCV` |
| `entry/ph_offset/sh_offset` | `uintptr_t` | ELF entry and offsets |
| `valid(Machine)` | `[[nodiscard]] bool` | Validate ELF header |

#### `Ph` — Program Header
| Member | Type | Description |
|---|---|---|
| `type/flags` | `uint32_t` | Segment type and flags |
| `f_offs/v_addr/p_addr/f_size/m_size/align` | `uint64_t` | Segment layout |

---

## 16. AArch64 Architecture-Specific Classes

> All classes in this section live under `inc/aarch64/` (and `src/aarch64/`) unless otherwise noted.

---

### `Sys_regs` — General-Purpose Registers
**File**: [../inc/aarch64/regs.hpp](../inc/aarch64/regs.hpp)

| Member | Type | Description |
|---|---|---|
| `gpr[31]` | `uint64_t` | X0–X30 general-purpose registers |

---

### `Exc_regs` — Exception Register File
**File**: [../inc/aarch64/regs.hpp](../inc/aarch64/regs.hpp)

Inherits `Sys_regs` and adds EL0/EL2 state:

| Sub-struct | Members | Description |
|---|---|---|
| `el0` | `sp`, `tpidr`, `tpidrro` | EL0 stack pointer and thread-ID registers |
| `el2` | `elr`, `spsr`, `esr`, `far` | EL2 link register, saved PSTATE, exception syndrome, fault address |

---

### `Cpu_regs` — Full CPU Register Context
**File**: [../inc/aarch64/regs.hpp](../inc/aarch64/regs.hpp)

Inherits `Exc_regs` and adds virtualisation state:

| Member | Type | Description |
|---|---|---|
| `vmcb` | `Vmcb *` | Pointer to Virtual Machine Control Block |
| `obj` | `Refptr<Space_obj>` | Object capability space for this context |
| `hst` | `Refptr<Space_hst>` | Host memory space |
| `gst` | `Refptr<Space_gst>` | Guest memory space (aarch64-specific; absent on RISC-V) |
| `hazard` | `Atomic<Hazard_t>` | Per-CPU hazard flags |

Two constructors:
- `Cpu_regs()` — default (idle EC)
- `Cpu_regs (Pd *, Vmcb *, Space_obj *, Space_hst *, Space_gst *)` — full initialisation

---

### `Sys_abi` — System Call ABI Mapping
**File**: [../inc/aarch64/abi.hpp](../inc/aarch64/abi.hpp)

Inherits `Sys_regs`. Maps hypercall parameters to AArch64 registers:

| Method | Maps to | Description |
|---|---|---|
| `p0()` | `gpr[0]` (X0) | First parameter / return value |
| `p1()` | `gpr[1]` (X1) | Second parameter |
| `p2()` | `gpr[2]` (X2) | Third parameter |
| `p3()` | `gpr[3]` (X3) | Fourth parameter |
| `p4()` | `gpr[4]` (X4) | Fifth parameter |
| `flags()` | bits [7:4] of `p0()` | Hypercall flags (extracted from X0) |

---

### `Ec` (AArch64 Specialisation)
**File**: [../inc/aarch64/ec_arch.hpp](../inc/aarch64/ec_arch.hpp)

Extends the architecture-independent `Ec` (from `inc/ec.hpp`):

| Element | Description |
|---|---|
| `needs_pio = false` | AArch64 has no I/O port space |
| `make_current(cont_t)` | Resets stack to `DSTK_TOP`, sets `current = this`, invokes continuation |
| `ret_user_hypercall` | Continuation: return to user after hypercall |
| `ret_user_exception` | Continuation: return to user after exception |
| `ret_user_vmexit` | Continuation: return to VMM after guest VM exit |
| `set_vmm_regs` | Continuation: copy UTCB data into VMM regs |
| `handle_irq(Ec *)` | Static: handles external interrupt during EC execution |
| `handle_exc(Ec *)` | Static: handles synchronous exception |

---

### `Cpu` (AArch64 Specialisation)
**File**: [../inc/aarch64/cpu.hpp](../inc/aarch64/cpu.hpp), [../src/aarch64/cpu.cpp](../src/aarch64/cpu.cpp)

| Element | Type | Description |
|---|---|---|
| `id` | `cpu_t` | Logical CPU index |
| `bsp` | `bool` | `true` for bootstrap processor |
| `mpidr` | `uint64_t` | MPIDR_EL1 affinity value |
| `hcr` | `uint64_t CPULOCAL` | HCR_EL2 value (computed from features) |
| `cptr` | `uint64_t CPULOCAL` | CPTR_EL2 value |
| `mdcr` | `uint64_t CPULOCAL` | MDCR_EL2 value |
| `res0_hcr/hcrx/cptr/mdcr` | `uint64_t` | RES0 masks per feature enumeration |
| `enumerate_features()` | `void` | Reads ID_AA64* registers, computes feature masks |
| `init(cpu)` | `static void` | Per-CPU init: features → GIC → Timer → Nptp → Vmcb |
| `allocate(cpu, mpidr, gicr)` | `static void` | Creates per-CPU page tables and data structures |
| `boot_cpu` | `static cpu_t` | BSP CPU number |

**Feature Enumeration** reads these ID registers:
`ID_AA64PFR0/1_EL1`, `ID_AA64DFR0/1_EL1`, `ID_AA64ISAR0/1/2_EL1`, `ID_AA64MMFR0/1/2/3/4_EL1`

---

### `Space_hst` (AArch64)
**File**: [../inc/aarch64/space_hst.hpp](../inc/aarch64/space_hst.hpp)

Host address space using stage-2 translation:

| Member | Type | Description |
|---|---|---|
| `vmid` | `Vmid` | Virtual Machine Identifier for this space |
| `nptp` | `Nptp` | Nested (stage-2) page table pointer |
| `nova` | `static Space_hst` | Kernel host space |
| `sbw` | `constexpr uint8_t` | `Npt::ibits - PAGE_BITS` (space bit-width) |

| Method | Description |
|---|---|
| `make_current()` | Writes `VTTBR_EL2` via `nptp.make_current(vmid)` |
| `update()` | Delegates to `nptp.update()` |
| `delegate()` | Maps pages from source space |
| `sync()` | TLB invalidation via VMID |

---

### `Space_gst` (AArch64)
**File**: [../inc/aarch64/space_gst.hpp](../inc/aarch64/space_gst.hpp)

Guest address space (aarch64 fully functional, unlike RISC-V stub):

| Member | Type | Description |
|---|---|---|
| `vmid` | `Vmid` | Guest VMID |
| `nptp` | `Nptp` | Guest stage-2 page table |

| Method | Description |
|---|---|
| `make_current()` | Activates guest stage-2 via VTTBR_EL2 |
| `update()` | Updates stage-2 mappings |
| `sync()` | TLB invalidation |

---

### `Space_dma` (AArch64)
**File**: [../inc/aarch64/space_dma.hpp](../inc/aarch64/space_dma.hpp)

DMA remapping space for SMMU:

| Member | Type | Description |
|---|---|---|
| `sdid` | `Sdid` | SMMU Domain Identifier |
| `dptp` | `Dptp` | DMA page table pointer |

| Method | Description |
|---|---|
| `update()` | Updates DMA mappings |
| `sync()` | `Smmu::tlb_invalidate_all(sdid)` |
| `get_sdid()` | Returns the SDID |
| `mco()` | Returns `Dpt::lev_ord()` (mapping-cache order) |

---

### `Vmcb` — Virtual Machine Control Block
**File**: [../inc/aarch64/vmcb.hpp](../inc/aarch64/vmcb.hpp)

Buddy-allocated (≤ `PAGE_SIZE`), stores guest VM state for world-switch:

| Sub-struct | Members | Description |
|---|---|---|
| `el1` | `afsr0`, `afsr1`, `amair`, `cntkctl`, `contextidr`, `cpacr`, `csselr`, `elr`, `esr`, `far`, `mair`, `mdscr`, `par`, `sctlr`, `sp`, `spsr`, `tcr`, `tpidr`, `ttbr0`, `ttbr1`, `vbar` (21 regs) | Guest EL1 system registers |
| `el2` | `hcr`, `hcrx`, `hpfar`, `vdisr`, `vmpidr`, `vpidr` | EL2 control and virtual registers |
| `a32` | `dacr`, `fpexc`, `hstr`, `ifsr`, `spsr_abt`, `spsr_fiq`, `spsr_irq`, `spsr_und` | AArch32 guest state |
| `tmr` | `cntvoff`, `cntv_cval`, `cntv_ctl`, `cntv_act` | Virtual timer state |
| `gic` | `lr[16]`, `ap0r[4]`, `ap1r[4]`, `elrsr`, `vmcr`, `hcr` | Virtual GIC list registers |

| Method | Description |
|---|---|
| `save_tmr()` | Reads `CNTVOFF_EL2`, `CNTV_CVAL_EL0`, `CNTV_CTL_EL0` |
| `load_tmr()` | Writes virtual timer registers from VMCB |
| `operator new` | Buddy allocation |
| `operator delete` | Buddy deallocation |

---

### `Vmid` — Virtual Machine Identifier
**File**: [../inc/aarch64/vmid.hpp](../inc/aarch64/vmid.hpp)

| Member | Type | Description |
|---|---|---|
| `val` | `uint16_t const` | Allocated VMID value |
| `allocator` | `static Atomic<uint16_t>` | Monotonic allocator |
| `operator auto()` | `val & 0xff` | Returns 8-bit VMID |

Used by `Space_hst` and `Space_gst` for tagging TLB entries via `VTTBR_EL2`.

---

### `Sdid` — SMMU Domain Identifier
**File**: [../inc/sdid.hpp](../inc/sdid.hpp)

| Member | Type | Description |
|---|---|---|
| `val` | `uint8_t const` | Allocated SDID value |
| `allocator` | `static Atomic<uint8_t>` | Monotonic allocator |
| `operator auto()` | `val` | Returns 8-bit SDID |

Used by `Space_dma` for SMMU domain isolation.

---

### `Utcb_arch` — User Thread Control Block (AArch64 Layout)
**File**: [../inc/aarch64/utcb_arch.hpp](../inc/aarch64/utcb_arch.hpp)

Total size: `0x2E8` bytes (744 bytes). Sections mirror VMCB + GPRs:

| Section | Key Members | Description |
|---|---|---|
| `el0` | `gpr[31]`, `sp`, `tpidr`, `tpidrro` | User-mode GPRs and thread-ID |
| `a32` | `spsr_abt/fiq/irq/und`, `dacr`, `ifsr`, `hstr` | AArch32 banked state |
| `el1` | `sp`, `tpidr`, `contextidr`, `elr`, `spsr`, `esr`, `far`, `afsr0/1`, `ttbr0/1`, `tcr`, `mair`, `amair`, `vbar`, `sctlr`, `mdscr` | Guest EL1 sysregs |
| `el2` | `hcr`, `hcrx`, `vpidr`, `vmpidr`, `elr`, `spsr`, `esr`, `far`, `hpfar` | EL2 control/trap regs |
| `tmr` | `cntv_cval`, `cntv_ctl`, `cntkctl`, `cntvoff` | Virtual timer state |
| `gic` | `lr[16]`, `ap0r[4]`, `ap1r[4]`, `elrsr`, `vmcr` | Virtual GIC list registers |
| `sel` | `gst` | Selector for guest space |

| Method | Description |
|---|---|
| `load(Mtd_arch, Cpu_regs &)` | Load fields specified by MTD bits from UTCB into CPU regs |
| `save(Mtd_arch, Cpu_regs &, Space_obj *)` | Save fields specified by MTD bits from CPU regs into UTCB |

---

### `Mtd_arch` — Message Transfer Descriptor (AArch64 Bits)
**File**: [../inc/aarch64/mtd_arch.hpp](../inc/aarch64/mtd_arch.hpp)

Bit-field enum selecting which register groups to transfer:

| Bit | Name | Description |
|---|---|---|
| 0 | `POISON` | Poison (invalid) |
| 1 | `ICI` | Instruction cache invalidation |
| 2 | `GPR` | General-purpose registers X0–X30 |
| 3 | `FPR` | Floating-point registers |
| 4 | `EL0_SP` | EL0 stack pointer + TPIDR |
| 5 | `EL0_IDR` | EL0 TPIDRRO |
| 7 | `A32_SPSR` | AArch32 banked SPSRs |
| 8 | `A32_DIH` | AArch32 DACR, IFSR, HSTR |
| 10–20 | `EL1_SP` .. `EL1_MDSCR` | EL1 system registers (SP, TPIDR, CTX, ELR, SPSR, ESR, FAR, AFSR, TTBR, TCR/MAIR/AMAIR, VBAR/SCTLR, MDSCR) |
| 23–27 | `EL2_HCR` .. `EL2_HPFAR` | EL2 control registers (HCR/HCRX, VPIDR/VMPIDR, ELR/SPSR, ESR/FAR, HPFAR) |
| 29 | `TMR` | Virtual timer state |
| 30 | `GIC` | Virtual GIC list registers |
| 31 | `SPACES` | Space selectors |

---

### Page Tables (AArch64)

#### `Hpt` — Host Page Table Entry
**File**: [../inc/aarch64/ptab_hpt.hpp](../inc/aarch64/ptab_hpt.hpp)

Stage-1 EL2 translation (`TTBR0_EL2`):

| Constant | Value | Description |
|---|---|---|
| `ibits` | 48 | Input address bits (256 TiB VA) |
| `lev()` | 4 | Page table levels |

| Attribute | Description |
|---|---|
| `ATTR_P` | Present |
| `ATTR_nL` | Not Large (table, not block) |
| `ATTR_nS` | Not Secure |
| `ATTR_U` | User accessible |
| `ATTR_nW` | Not Writable |
| `ATTR_A` | Accessed |
| `ATTR_nG` | Not Global |
| `ATTR_nX` | Not Executable |

#### `Hptp` — Host Page Table Pointer
| Member | Description |
|---|---|
| `master` | Static kernel page table |
| `make_current()` | Writes `TTBR0_EL2`, issues `isb` |
| `invalidate_cpu()` | `tlbi alle2; dsb nsh; isb` |
| `share_from_master()` | Copies top-level entries from master |
| `map()` / `map_tmp()` | Static mapping helpers |

#### `Npt` — Nested Page Table Entry
**File**: [../inc/aarch64/ptab_npt.hpp](../inc/aarch64/ptab_npt.hpp)

Stage-2 translation (`VTTBR_EL2` + VMID):

| Constant | Value | Description |
|---|---|---|
| `ibits` | 48 (SERVER) / 40 (non-SERVER) | Input address bits |
| `lev()` | 4 | Page table levels |

| Attribute | Description |
|---|---|
| `ATTR_P` | Present |
| `ATTR_nL` | Not Large |
| `ATTR_R` | Readable |
| `ATTR_W` | Writable |
| `ATTR_A` | Accessed |
| `ATTR_nX0/nX1` | Not Executable (dual XN bits) |
| `ATTR_K` | Kernel Memory |

#### `Nptp` — Nested Page Table Pointer
| Member | Description |
|---|---|
| `make_current(Vmid)` | Writes VTTBR_EL2 = `(VMID << 48) | root_addr` |

#### `Dpt` — DMA Page Table Entry
**File**: [../inc/aarch64/ptab_dpt.hpp](../inc/aarch64/ptab_dpt.hpp)

SMMU stage-2 translation:

| Constant | Value | Description |
|---|---|---|
| `ibits` | 48 | Input address bits |
| `lev()` | 4 | Page table levels |
| `noncoherent` | `static bool` | Whether SMMU is non-coherent |

| Attribute | Description |
|---|---|
| `ATTR_P` | Present |
| `ATTR_nL` | Not Large |
| `ATTR_R/W` | Readable/Writable |
| `ATTR_A` | Accessed |
| `ATTR_ORA/OWA` | Override Read/Write-Allocate |

#### `Pte` — Page Table Entry Base (Arm)
**File**: [../inc/aarch64/ptab_pte.hpp](../inc/aarch64/ptab_pte.hpp)

| Method | Description |
|---|---|
| `type(l)` | Returns `PTAB`, `LEAF`, or `HOLE` based on `ATTR_nL` and level |
| `publish()` | `DSB ISH` — ensures PTE visibility |
| `pas(e)` | Decodes Physical Address Size from ID register encoding |

---

### GIC Components

#### `Gicd` — GIC Distributor
**File**: [../inc/aarch64/gicd.hpp](../inc/aarch64/gicd.hpp)

| Element | Description |
|---|---|
| `init()` | Static: Configures SPI routing, priority, enable |
| Inherits | `Coresight`, `Intid`, `Mmio` (for MMIO register access) |

#### `Gicr` — GIC Redistributor
**File**: [../inc/aarch64/gicr.hpp](../inc/aarch64/gicr.hpp)

| Element | Description |
|---|---|
| `init()` | Static: Configures per-PE redistributor, wakes PE |
| Inherits | `Coresight`, `Intid`, `Mmio` |

#### `Gicc` — GIC CPU Interface
**File**: [../inc/aarch64/gicc.hpp](../inc/aarch64/gicc.hpp)

| Element | Description |
|---|---|
| `init()` | Static: Configures ICC_* system registers |
| `ack()` | Reads `ICC_IAR1_EL1` (acknowledge interrupt) |
| `eoi()` | Writes `ICC_EOIR1_EL1` (end of interrupt) |

#### `Gich` — GIC Hypervisor Interface
**File**: [../inc/aarch64/gich.hpp](../inc/aarch64/gich.hpp)

| Element | Description |
|---|---|
| `init()` | Static: Configures ICH_* system registers |
| `num_lr` | Number of list registers (from `ICH_VTR_EL2`) |

#### `Gits` — GIC Interrupt Translation Service
**File**: [../inc/aarch64/gits.hpp](../inc/aarch64/gits.hpp)

| Element | Description |
|---|---|
| `initialize()` | Static: iterates linked list of ITS instances, initialises each |
| Inherits | `Coresight`, `Intid`, `Mmio`, `List<Gits>` |
| Purpose | Translates DeviceID + EventID → INTID for MSI/LPI routing |

---

### `Smmu` — System Memory Management Unit
**File**: [../inc/aarch64/smmu.hpp](../inc/aarch64/smmu.hpp)

Abstract base class; derived into `Smmu_v2` and `Smmu_v3`:

| Element | Description |
|---|---|
| `init()` | Virtual: per-instance SMMU initialisation |
| `assign_dev()` | Virtual: assigns a device stream to a DMA space |
| `initialize()` | Static: iterates linked list, calls `init()`, sets HIP SMMU feature |
| `tlb_invalidate_all(Sdid)` | Static: invalidates SMMU TLB for a domain |
| `using_iid(iid)` | Static: checks if an interrupt ID is used by SMMU |
| Inherits | `List<Smmu>` (linked list of SMMU instances) |

---

### `Interrupt` — Interrupt Handling
**File**: [../inc/aarch64/interrupt.hpp](../inc/aarch64/interrupt.hpp)

| Member | Type | Description |
|---|---|---|
| `table_s[NUM_SPI]` | `Refptr<Sm>` | Semaphores bound to SPIs |
| `table_e[NUM_ESPI]` | `Refptr<Sm>` | Semaphores bound to Extended SPIs |
| `table_l[NUM_LPI]` | `Refptr<Sm>` | Semaphores bound to LPIs |
| `guest_s` | `Bitmap<NUM_SPI>` | SPI guest-ownership bitmap |
| `guest_e` | `Bitmap<NUM_ESPI>` | ESPI guest-ownership bitmap |
| `num_spi/eppi/espi/lpi` | `unsigned` | Interrupt counts (filled by GIC discovery) |

| Method | Description |
|---|---|
| `handler(bool)` | Dispatches to `handle_sgi/ppi/spi/eppi/espi/lpi` based on INTID |
| `assign(...)` | Binds an Sm to an interrupt, configures routing |
| `deactivate(Sm *)` | Deactivates an interrupt |
| `send_cpu(Request, cpu_t)` | Sends SGI to a specific CPU |
| `send_exc(Request)` | Sends SGI for exception handling |
| `get_ptr(iid)` | Returns `Refptr<Sm> *` for the given interrupt ID |

Interrupt types:
- **SGI** (0–15): Software Generated Interrupts — IPIs
- **PPI** (16–31): Private Peripheral Interrupts (timer, PMU)
- **SPI** (32–1019): Shared Peripheral Interrupts
- **ESPI** (4096–5119): Extended SPIs (GICv3.1)
- **LPI** (8192+): Locality-specific Peripheral Interrupts (MSI)

---

### `Timer` — Architecture Timer
**File**: [../inc/aarch64/timer.hpp](../inc/aarch64/timer.hpp)

Inherits `Stc` (System Time Counter):

| Member | Type | Description |
|---|---|---|
| `offs` | `static uint64_t` | Skew between physical and system time |
| `ppi_el2_p` | `static unsigned` | PPI number for EL2 physical timer |
| `ppi_el1_v` | `static unsigned` | PPI number for EL1 virtual timer |
| `lvl_el2_p/lvl_el1_v` | `static bool` | Level-triggered flags |

| Method | Description |
|---|---|
| `time()` | Returns current system time (reads `CNTPCT_EL0` − offset) |
| `set_dln(s)` | Sets deadline via `CNTHP_CVAL_EL2` |
| `stop()` | Sets `CNTHP_CVAL_EL2` to `~0ULL` (infinity) |
| `phys_to_syst(p)` | Convert physical → system time |
| `syst_to_phys(s)` | Convert system → physical time |
| `init()` | Static: configure timer |

---

### `Fpu` — Floating-Point Unit
**File**: [../inc/aarch64/fpu.hpp](../inc/aarch64/fpu.hpp)

| Member | Type | Description |
|---|---|---|
| `regs.v[32][2]` | `uint64_t` | 32 × 128-bit SIMD/FP registers (Q0–Q31) |
| `regs.fpcr` | `uint64_t` | Floating-Point Control Register |
| `regs.fpsr` | `uint64_t` | Floating-Point Status Register |
| `size` | `constexpr 528` | Total context size in bytes |
| `alignment` | `constexpr 16` | Alignment requirement |

| Method | Description |
|---|---|
| `load()` | Restores SIMD/FP state from memory via `ldp q` pairs |
| `save()` | Saves SIMD/FP state to memory via `stp q` pairs |
| `disable()` | Sets `CPTR_EL2.TFP`, clears FPU hazard |
| `enable()` | Clears `CPTR_EL2.TFP`, sets FPU hazard |
| `fini()` | FPU finalisation |
| `operator new/delete` | Slab-allocated |

---

### `Smc_arch` — SMC Architecture Calls
**File**: [../inc/aarch64/smc_arch.hpp](../inc/aarch64/smc_arch.hpp)

Inherits `Smc` (Secure Monitor Call base):

| Element | Description |
|---|---|
| `Status` | `SUCCESS`, `NOT_SUPPORTED`, `NOT_REQUIRED`, `INVALID_PARAMETER` |
| `Function32` | `VERSION`, `FEATURES`, `SOC_ID`, `FEATURE_AVAILABILITY`, `WORKAROUND_1/2/3/4` |
| `version()` | Queries SMCCC version |
| `features(id)` | Queries feature availability |
| `workarounds` | `static unsigned` — bitmask of available CPU errata workarounds |
| `init()` | Static: probes SMCCC features and workarounds |

---

### `Event` — Event Selectors
**File**: [../inc/aarch64/event.hpp](../inc/aarch64/event.hpp)

| Constant | Value | Description |
|---|---|---|
| `NONE` | -1 | No event |
| `STARTUP` | 0 | EC startup event |
| `RECALL` | 1 | EC recall event |
| `VTIMER` | 2 | Virtual timer event |
| `hst_arch` | 64 | Architecture host event selector base |
| `gst_arch` | 64 | Architecture guest event selector base |
| `hst_max` | `1 + RECALL` = 2 | Max host event selectors |
| `gst_max` | `1 + VTIMER` = 3 | Max guest event selectors |

---

### `Hip_arch` — Hypervisor Information Page (AArch64 Part)
**File**: [../inc/aarch64/hip_arch.hpp](../inc/aarch64/hip_arch.hpp)

| Member | Type | Description |
|---|---|---|
| `num_spi` | `uint16_t` | Number of SPIs |
| `num_espi` | `uint16_t` | Number of Extended SPIs |
| `num_lpi` | `uint32_t` | Number of LPIs |
| `num_smg` | `uint16_t` | Number of SMMU Stream Mapping Groups |
| `num_ctx` | `uint16_t` | Number of SMMU Context Banks |
| `Feature::SMMU` | `BIT(0)` | SMMU presence feature flag |
| `build()` | `void` | Fills fields from `Interrupt` and `Smmu` counts |

`static_assert` enforces standard layout and `sizeof(Hip_arch) == 16`.

---

*End of class documentation.*
