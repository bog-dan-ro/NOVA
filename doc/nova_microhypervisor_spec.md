# NOVA Microhypervisor Interface Specification

**Author:** Udo Steinberg <udo@hypervisor.org>  
**Date:** December 1, 2025

---

*Copyright © 2006–2011 Udo Steinberg, Technische Universität Dresden*  
*Copyright © 2012–2013 Udo Steinberg, Intel Corporation*  
*Copyright © 2014–2016 Udo Steinberg, FireEye, Inc.*  
*Copyright © 2019–2025 Udo Steinberg, BlueRock Security, Inc.*

> This specification is provided "as is" and may contain defects or deficiencies which cannot or will not be corrected. The author makes no representations or warranties, either expressed or implied, including but not limited to, warranties of merchantability, fitness for a particular purpose, or non-infringement that the contents of the specification are suitable for any purpose or that any practice or implementation of such contents will not infringe any third party patents, copyrights, trade secrets or other rights.

---

## Notation

The key words **must**, **must not**, **required**, **should**, **should not**, **recommended**, **may** and **optional** in this document are to be interpreted as described in RFC 2119.

Throughout this document, the following symbols are used:

- `∼` — The value of this parameter or field is **unknown**. The microhypervisor cannot ensure that the value does not leak information across protection domain boundaries.
- `/` — The value of this parameter or field is **undefined**. The microhypervisor ensures that the value does not leak information across protection domain boundaries. Future versions of this specification may define a value for the parameter or field.
- `–` — The value of this parameter or field is **ignored**. Future versions of this specification may define a meaning for the parameter or field.
- `≡` — The value of this parameter or field is **unchanged**. The microhypervisor preserves the value across hypercalls.

---

# Part I: Introduction

## 1. System Architecture

The **NOVA OS Virtualization Architecture** (NOVA) facilitates the coexistence of multiple legacy guest operating systems and a user-mode host framework on a single platform. The core system leverages hardware virtualization technology provided by modern x86 or Arm platforms and comprises the NOVA microhypervisor and one or more Virtual-Machine Monitors (VMMs).

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                                                                             │
│  VM              VM              VM              VM                         │
│  ┌───────────┐   ┌─────────────┐ ┌─────────────┐ ┌───────────┐   ...       │
│  │   Guest   │   │  Container  │ │   Virtual   │ │ Unikernel │             │
│  │Operating  │   │  Runtime    │ │  Appliance  │ │           │     guest   │
│  │  System   │   │             │ │             │ │           │             │
│  └─────┬─────┘   └──────┬──────┘ └──────┬──────┘ └─────┬─────┘            │
├ ─ ─ ─ ─│─ ─ ─ ─ ─ ─ ─ ─│─ ─ ─ ─ ─ ─ ─ │─ ─ ─ ─ ─ ─ ─│─ ─ ─ ─ ─ ─ ─ ─ ┤
│         ▼               ▼               ▼               ▼                  │
│       VMM             VMM             VMM             VMM          ...  host│
│  ┌──────────────────────────────────────────────────────────────────────┐   │
│  │  Applications   │  Root Partition Manager  │  Drivers              │   │
│  └──────────────────────────────────────────────────────────────────────┘   │
│                                                                       user  │
├─────────────────────────────────────────────────────────────────────────────┤
│                        Microhypervisor                                kernel│
└─────────────────────────────────────────────────────────────────────────────┘
```

*Figure 1.1: System Architecture*

The microhypervisor is the only component executing in privileged host/kernel mode. It isolates the various user-mode components, including the virtual-machine monitors, from one another by placing them in different protection domains in unprivileged host/user mode. Each legacy guest operating system runs in its own virtual-machine environment in guest mode and is therefore isolated from the other components.

Besides spatial and temporal isolation, the microhypervisor also provides mechanisms for partitioning and delegation of platform resources, such as CPU time, physical memory, I/O ports and hardware interrupts and for establishing communication channels and signaling between different protection domains.

The virtual-machine monitors handle virtualization events and implement virtual devices that enable legacy guest operating systems to function in the same manner as they would on bare-metal hardware. Providing this functionality outside the microhypervisor in the VMMs reduces the size of the trusted computing base significantly for all components that do not require virtualization support.

---

# Part II: Basic Abstractions

## 2. Kernel Objects

### 2.1 Protection Domain

1. The **Protection Domain (PD)** is a unit of protection and spatial isolation.
2. Access to a Protection Domain is controlled by a **PD Capability** (`CAP_OBJ_PD`).
3. Different types of spaces can be created for a PD.
4. Spaces store Capabilities (CAPs) for kernel objects or platform resources that Execution Contexts in that Protection Domain can access.

#### 2.1.1 Object Space

1. Only one **Object Space** (`SPC_OBJ`) can be created per Protection Domain.
2. Access to an Object Space is controlled by an **Object Space Capability** (`CAP_OBJ_OBJ`).
3. Each hypercall invoked by a Host Execution Context explicitly specifies an Object Capability Selector to designate the kernel object on which it operates.
4. The **Object Capability Selector** (`SEL_OBJ`) serves as index into the EC's Object Space and selects a slot that contains either a Null Capability (`CAP0`) or an Object Capability (`CAP_OBJ`) that refers to a kernel object with associated access permissions.

#### 2.1.2 Host Space

1. Only one **Host Space** (`SPC_HST`) can be created per Protection Domain.
2. Access to a Host Space is controlled by a **Host Space Capability** (`CAP_OBJ_HST`).
3. Each memory operation issued by a Host Execution Context implicitly uses the page number of the accessed Host-Virtual Address (HVA) as Host Capability Selector: `SEL_HST = HVA >> 12`.
4. The **Host Capability Selector** (`SEL_HST`) serves as index into the EC's Host Space and selects a slot that contains either a Null Capability (`CAP0`) or a Memory Capability (`CAP_MEM`) that refers to a 4 KiB page frame in physical memory with associated access permissions.

#### 2.1.3 Guest Space

1. Multiple **Guest Spaces** (`SPC_GST`) can be created per Protection Domain.
2. Access to a Guest Space is controlled by a **Guest Space Capability** (`CAP_OBJ_GST`).
3. Each memory operation issued by a Guest Execution Context implicitly uses the page number of the accessed Guest-Physical Address (GPA) as Guest Capability Selector: `SEL_GST = GPA >> 12`.
4. The **Guest Capability Selector** (`SEL_GST`) serves as index into the EC's Guest Space and selects a slot containing either `CAP0` or a `CAP_MEM` referring to a 4 KiB page frame.

#### 2.1.4 DMA Space

1. Multiple **DMA Spaces** (`SPC_DMA`) can be created per Protection Domain.
2. Access to a DMA Space is controlled by a **DMA Space Capability** (`CAP_OBJ_DMA`).
3. Each DMA operation issued by a device implicitly uses the page number of the accessed DMA-Virtual Address (DVA) as DMA Capability Selector: `SEL_DMA = DVA >> 12`.
4. The **DMA Capability Selector** (`SEL_DMA`) serves as index into the device's DMA Space and selects a slot containing either `CAP0` or a `CAP_MEM`.

#### 2.1.5 PIO Space

1. Multiple **PIO Spaces** (`SPC_PIO`) can be created per Protection Domain.
2. Access to a PIO Space is controlled by a **PIO Space Capability** (`CAP_OBJ_PIO`).
3. Each PIO access (IN/OUT instruction) issued by an Execution Context (EC) implicitly uses the number of the accessed I/O port as PIO Capability Selector.
4. The **PIO Capability Selector** (`SEL_PIO`) serves as index into the EC's PIO Space.

#### 2.1.6 MSR Space

1. Multiple **MSR Spaces** (`SPC_MSR`) can be created per Protection Domain.
2. Access to an MSR Space is controlled by an **MSR Space Capability** (`CAP_OBJ_MSR`).
3. Each MSR access (RDMSR/WRMSR instruction) issued by an Execution Context (EC) implicitly uses the number of the accessed MSR as MSR Capability Selector.
4. The **MSR Capability Selector** (`SEL_MSR`) serves as index into the EC's MSR Space.

---

### 2.2 Execution Context

1. The **Execution Context (EC)** is an abstraction for an activity within a PD.
2. Access to an Execution Context is controlled by an **EC Capability** (`CAP_OBJ_EC`).
3. An EC is permanently bound to one CPU.
4. An EC contains architecture-dependent state, such as CPU registers and (optionally) FPU registers.

#### 2.2.1 Host Execution Context

1. There exist two types of **Host Execution Context** (`EC_HST`):
   - **Local Threads** – may have PTs (but no SCs) bound to it.
   - **Global Threads** – may have an SC (but no PTs) bound to it.
2. A Host Execution Context has a UTCB that enables it to perform regular IPC.
3. Upon creation, a Host Execution Context is permanently bound to: the first Object Space, the first Host Space, and the first PIO Space†.
4. A Host Execution Context cannot be reassigned to any spaces.

#### 2.2.2 Guest Execution Context

1. There exists one type of **Guest Execution Context** (`EC_GST`):
   - **Virtual CPUs** – may have an SC (but no PTs) bound to it.
2. A Guest Execution Context does not have a UTCB.
3. Upon creation, a Guest Execution Context is permanently bound to: the first Object Space and the first Host Space.
4. Upon any architectural/microhypervisor event, a Guest Execution Context may be (re)assigned to: any Guest Space†, any PIO Space†, any MSR Space†.

> † Only on architectures where this type of space is available.

---

### 2.3 Scheduling Context

1. The **Scheduling Context (SC)** is a unit of prioritization and temporal isolation.
2. Access to a Scheduling Context is controlled by an **SC Capability** (`CAP_OBJ_SC`).
3. An SC is permanently bound to one CPU.
4. An SC is permanently bound to the EC for which it was created.
5. Donation allows another EC to consume the budget of the SC for the duration of the donation.
6. A scheduling context comprises:
   - Reference to bound EC
   - Class Of Service (COS)
   - Priority – numerically higher priorities always preempt numerically lower priorities
   - Budget – time after which the SC can be preempted by an SC with the same priority

### 2.4 Portal

1. A **Portal (PT)** represents a dedicated entry point into the PD for which the portal was created.
2. Access to a Portal is controlled by a **PT Capability** (`CAP_OBJ_PT`).
3. A PT is permanently bound to the EC for which it was created.
4. A portal comprises: Reference to bound EC, Message Transfer Descriptor (MTD), Entry Instruction Pointer (IP), Portal Identifier (PID).

### 2.5 Semaphore

1. A **Semaphore (SM)** provides a means to synchronize execution and interrupt delivery by selectively blocking and unblocking Execution Contexts (ECs).
2. Access to a Semaphore is controlled by an **SM Capability** (`CAP_OBJ_SM`).

### 2.6 Device Context

1. A **Device Context (DC)** provides management information for a hardware device.
2. Access to a Device Context is controlled by a **DC Capability** (`CAP_OBJ_DC`).
3. A Device Context comprises: topology of the device (PCI SBDF), reference to the IOMMU/SMMU for DMA translation, reference to the IOMMU/GITS for MSI translation, and auxiliary information.

---

## 3. Hardware Resources

### 3.1 System Time Counter

The system time is represented by an unsigned 64-bit **System Time Counter (STC)** with the following properties:

1. The STC starts with a power-on value of 0.
2. Subsequent reads of the STC return a higher value that reflects the platform uptime.
3. While the platform is in a shallow sleep state, the STC retains its current value.
4. While the platform is running, the STC monotonically increments at a fixed frequency, conveyed in the Hypervisor Information Page (HIP).
5. The STC and its frequency are synchronized across all CPUs.
6. Applications can obtain the current STC value as follows:
   - **Arm:** By reading `CNTVCT_EL0` via the `MRS` instruction.
   - **x86:** By reading `IA32_TSC` via the `RDTSC` instruction.

---

# Part III: Application Programming Interface

## 4. Data Types

### 4.1 Capability

Capabilities are communicable, unforgeable tokens of authority. They consist of a reference to a resource coupled with access permissions. Capabilities are opaque and immutable for applications — they cannot be inspected or modified directly; instead applications refer to a Capability (CAP) via a Capability Selector (SEL).

#### 4.1.1 Null Capability

A **Null Capability** (`CAP0`) does not refer to anything and carries no permissions.

#### 4.1.2 Object Capability

An **Object Capability** (`CAP_OBJ`) is stored in the Object Space of a PD and refers to a kernel object.

##### 4.1.2.1 Object Space Capability

```
Bits: [ 5..2: – ][ 1: TAKE ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that Object Space as destination.
- **TAKE** – If set, `ctrl_pd` can use that Object Space as source.

##### 4.1.2.2 Host Space Capability

```
Bits: [ 5..2: – ][ 1: TAKE ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that Host Space as destination.
- **TAKE** – If set, `ctrl_pd` can use that Host Space as source.

##### 4.1.2.3 Guest Space Capability

```
Bits: [ 5..3: – ][ 2: ASSIGN ][ 1: – ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that Guest Space as destination.
- **ASSIGN** – If set, `ipc_reply` can assign an `EC_GST` to that Guest Space.

##### 4.1.2.4 DMA Space Capability

```
Bits: [ 5..3: – ][ 2: ASSIGN ][ 1: – ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that DMA Space as destination.
- **ASSIGN** – If set, `assign_dev` can assign a device to that DMA Space.

##### 4.1.2.5 PIO Space Capability

```
Bits: [ 5..3: – ][ 2: ASSIGN ][ 1: TAKE ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that PIO Space as destination.
- **TAKE** – If set, `ctrl_pd` can use that PIO Space as source.
- **ASSIGN** – If set, `ipc_reply` can assign an `EC_GST` to that PIO Space.

##### 4.1.2.6 MSR Space Capability

```
Bits: [ 5..3: – ][ 2: ASSIGN ][ 1: TAKE ][ 0: GRANT ]
```

- **GRANT** – If set, `ctrl_pd` can use that MSR Space as destination.
- **TAKE** – If set, `ctrl_pd` can use that MSR Space as source.
- **ASSIGN** – If set, `ipc_reply` can assign an `EC_GST` to that MSR Space.

##### 4.1.2.7 Protection Domain Capability

```
Bits: [ 5: DC ][ 4: SM ][ 3: PT ][ 2: SC ][ 1: EC ][ 0: PD ]
```

- **PD** – If set, `create_pd` permitted.
- **EC** – If set, `create_ec` permitted.
- **SC** – If set, `create_sc` permitted.
- **PT** – If set, `create_pt` permitted.
- **SM** – If set, `create_sm` permitted.
- **DC** – If set, `create_dc` permitted.

##### 4.1.2.8 Execution Context Capability

```
Bits: [ 5..4: – ][ 3: BINDSC ][ 2: BINDPT ][ 1: – ][ 0: CTRL ]
```

- **CTRL** – If set, `ctrl_ec` permitted.
- **BINDPT** – If set, `create_pt` can bind a Portal (PT) to the EC.
- **BINDSC** – If set, `create_sc` can bind a Scheduling Context (SC) to the EC.

##### 4.1.2.9 Scheduling Context Capability

```
Bits: [ 5..1: – ][ 0: CTRL ]
```

- **CTRL** – If set, `ctrl_sc` permitted.

##### 4.1.2.10 Portal Capability

```
Bits: [ 5..3: – ][ 2: EVENT ][ 1: CALL ][ 0: CTRL ]
```

- **CTRL** – If set, `ctrl_pt` permitted.
- **CALL** – If set, `ipc_call` permitted.
- **EVENT** – If set, delivery of events permitted.

##### 4.1.2.11 Semaphore Capability

```
Bits: [ 5..3: – ][ 2: ASSIGN ][ 1: CTRLDN ][ 0: CTRLUP ]
```

- **CTRLUP** – If set, `ctrl_sm` (Up) permitted.
- **CTRLDN** – If set, `ctrl_sm` (Down) permitted.
- **ASSIGN** – †If set, `assign_int` permitted. (Only defined for interrupt semaphores.)

##### 4.1.2.12 Device Context Capability

```
Bits: [ 5..2: – ][ 1: ASSIGNINT ][ 0: ASSIGNDEV ]
```

- **ASSIGNDEV** – If set, `assign_dev` to a DMA Space permitted.
- **ASSIGNINT** – If set, `assign_int` to an Interrupt Semaphore permitted.

#### 4.1.3 Memory Capability

A **Memory Capability** (`CAP_MEM`) is stored in a Host/Guest/DMA Space and carries:

```
Bits: [ 5..4: – ][ 3: XS ][ 2: XU ][ 1: W ][ 0: R ]
```

- **R** – If set, the page frame is readable.
- **W** – If set, the page frame is writable.
- **XU** – If set, the page frame is executable (user mode).
- **XS** – If set, the page frame is executable (supervisor mode).

> ‡ If the hardware supports only combined execute permissions (X) for both modes, then X = XU ∨ XS.

#### 4.1.4 PIO Capability

A **PIO Capability** (`CAP_PIO`) is stored in a PIO Space and carries:

```
Bits: [ 5..1: – ][ 0: A ]
```

- **A** – If set, the I/O port is accessible (via IN/OUT).

#### 4.1.5 MSR Capability

An **MSR Capability** (`CAP_MSR`) is stored in an MSR Space and carries:

```
Bits: [ 5..2: – ][ 1: W ][ 0: R ]
```

- **R** – If set, the MSR is readable (via RDMSR).
- **W** – If set, the MSR is writable (via WRMSR).

---

### 4.2 Capability Selector

A **Capability Selector (SEL)** is an application-visible unsigned number that serves as an index into a space of an Execution Context to select a Capability (CAP).

| Selector | Space Available If | Notes |
|---|---|---|
| `SEL_OBJ` | `SBW_OBJ` ≠ 0 | Valid range: [0, 2^SBW_OBJ) |
| `SEL_HST` | `SBW_HST` ≠ 0 | Valid range: [0, 2^SBW_HST) → valid HVA range |
| `SEL_GST` | `SBW_GST` ≠ 0 | Valid range: [0, 2^SBW_GST) → valid GPA range |
| `SEL_DMA` | `SBW_DMA` ≠ 0 | Valid range: [0, 2^SBW_DMA) → valid DVA range |
| `SEL_PIO` | `SBW_PIO` ≠ 0 | Valid range: [0, 2^SBW_PIO) → valid PIO range |
| `SEL_MSR` | `SBW_MSR` ≠ 0 | Valid range: [0, 2^SBW_MSR) → valid MSR range |

PIO and MSR spaces are unavailable on non-x86 systems.

---

### 4.3 User Thread Control Block

A **User Thread Control Block (UTCB)** has a size of one memory page (4 KiB) and is mapped in the Host Space of its associated Host Execution Context. Because a UTCB is allocated and owned by the microhypervisor, it cannot be delegated using `ctrl_pd`.

#### 4.3.1 Regular Layout

During regular IPC, the UTCB is used for data transfer with 512 message words (`word0` through `word511`).

```
+0x1000  ┌──────────────────────────┐
         │        word511           │ +0x0ff8
         │        word510           │ +0x0ff0
         │           ...            │
         │        word1             │ +0x0008
         │        word0             │ +0x0000
         └──────────────────────────┘
                Message Words
```

- Data is copied from low words to high words, beginning with `word0`.
- Loads/stores are non-atomic with relaxed memory ordering.

#### 4.3.2 Architectural Layout

During architectural IPC (exceptions/intercepts), the UTCB is used for state transfer between architectural registers and the UTCB fields (architecture-specific layout for Arm / x86).

---

### 4.4 Message Transfer Descriptor

#### 4.4.1 Regular IPC

```
Bits [31..9]: –    Bits [8..0]: UTCB Message Words - 1
```

The MTD controls the data transfer:
- During `ipc_call`, it specifies the number of message words to transfer from the caller EC's UTCB to the callee EC's UTCB.
- During `ipc_reply`, it specifies the number of message words to transfer from the callee EC's UTCB back to the caller EC's UTCB.

```
Regular IPC Call:                     Regular IPC Reply:

 PD_A          PD_B                    PD_A          PD_B
 ┌──────┐      ┌──────┐                ┌──────┐      ┌──────┐
 │EC    │      │EC    │                │EC    │      │EC    │
 │caller│      │callee│                │caller│      │callee│
 │UTCB  │─────▶│UTCB  │                │UTCB  │◀─────│UTCB  │
 └──────┘  PT  └──────┘                └──────┘      └──────┘
 ipc_call(PT,MTD)                      ipc_reply(MTD)
```

#### 4.4.2 Architectural IPC

For exceptions and intercepts, the MTD uses an architectural bitfield layout where each bit controls transmission of specific architectural state.

```
Exception/Intercept:                  Return:

 PD_A          PD_B                    PD_A          PD_B
 ┌──────┐      ┌──────┐                ┌──────┐      ┌──────┐
 │EC    │  PT  │EC    │                │EC    │      │EC    │
 │affctd│─────▶│callee│                │affctd│◀─────│callee│
 │State │      │UTCB  │                │State │      │UTCB  │
 └──────┘      └──────┘                └──────┘      └──────┘
 implicit_call(PT, MTD_PT)             ipc_reply(MTD)
```

---

### 4.5 Scheduling Context Descriptor

```
Bits: [ 63..39: – ][ 38..23: COS ][ 22..16: Prio ][ 15..0: Budget ]
```

- **Budget** – Scheduling budget in milliseconds; must be > 0.
- **Prio** – Scheduling priority; must be > 0.
- **COS** – Class Of Service; must be 0 if COS not supported, or < `COS_NUM` if supported.

---

## 5. Hypercalls

### 5.1 Definitions

#### 5.1.1 Hypercall Numbers

| Number | Hypercall |
|--------|-----------|
| 0x0 | `ipc_call` |
| 0x1 | `ipc_reply` |
| 0x2 | `create_pd` |
| 0x3 | `create_ec` |
| 0x4 | `create_sc` |
| 0x5 | `create_pt` |
| 0x6 | `create_sm` |
| 0x7 | `create_dc` |
| 0x8 | `ctrl_pd` |
| 0x9 | `ctrl_ec` |
| 0xa | `ctrl_sc` |
| 0xb | `ctrl_pt` |
| 0xc | `ctrl_sm` |
| 0xd | `ctrl_hw` |
| 0xe | `assign_dev` |
| 0xf | `assign_int` |

#### 5.1.2 Status Codes

| Number | Status Code | Description |
|--------|-------------|-------------|
| 0x0 | `SUCCESS` | Operation Successful |
| 0x1 | `TIMEOUT` | Operation Timeout |
| 0x2 | `ABORTED` | Operation Abort |
| 0x3 | `OVRFLOW` | Operation Overflow |
| 0x4 | `BAD_HYP` | Invalid Hypercall |
| 0x5 | `BAD_CAP` | Invalid Capability |
| 0x6 | `BAD_PAR` | Invalid Parameter |
| 0x7 | `BAD_FTR` | Invalid Feature |
| 0x8 | `BAD_CPU` | Invalid CPU Number |
| 0x9 | `BAD_DEV` | Invalid Device ID |
| 0xa | `MEM_OBJ` | Insufficient Memory (Object Creation) |
| 0xb | `MEM_CAP` | Insufficient Memory (Capability Creation) |
| ≥0xc | — | reserved for future use |

---

### 5.2 Communication

#### 5.2.1 IPC Call

```c
status = ipc_call(SEL_OBJ pt,   // Portal
                  MTD &mtd)     // Message Transfer Descriptor
```

**Flags:** `[ 3..1: 0 ][ 0: T ]`

**Description:** Sends a message from `EC_CURRENT` (caller) to the EC (callee) to which the specified Portal (PT) is bound.

**Prior to the hypercall:**
- `SPC_OBJ_CURRENT[pt]` must refer to a `CAP_OBJ_PT` with permission `CALL`.

**If completed successfully:**
- If T=0 (No Timeout): If the callee EC was still busy, the caller EC helps run the prior `ipc_call` to completion.
- The microhypervisor transfers a message from the caller EC's UTCB to the callee EC's UTCB.
- The hypercall returns once the callee EC has invoked an `ipc_reply`.
- `SC_CURRENT` is donated to the callee EC upon `ipc_call` and returned upon `ipc_reply`.

**Status codes:** `SUCCESS`, `BAD_CAP`, `BAD_CPU`, `TIMEOUT` (if T=1), `ABORTED`

#### 5.2.2 IPC Reply

```c
pid = ipc_reply(MTD &mtd)  // Message Transfer Descriptor
```

**Flags:** `[ 3..0: 0 ]`

**Description:** Sends a reply message from `EC_CURRENT` (callee) back to the caller EC (if one exists) and subsequently waits for the next incoming message.

**If completed successfully:**
- If a caller EC exists: the microhypervisor transfers a reply message from the callee EC's UTCB back to the caller EC's UTCB, and `SC_CURRENT` is returned to the caller EC.
- `EC_CURRENT` blocks until the next incoming message arrives on any Portal (PT) bound to it.

**Returns:** This hypercall does not return directly. When the next message arrives, execution continues at the Instruction Pointer (IP) configured in the called PT.

---

### 5.3 Object Creation

#### 5.3.1 Create Protection Domain

```c
status = create_pd(SEL_OBJ sel,  // Created PD or Space
                   SEL_OBJ pd)   // Owner PD
```

**Flags:** `[ 3..2: – ][ 1..0: OP ]`

**Description:** Creates a new Protection Domain (PD) or an empty new space for a Protection Domain.

| OP | Result |
|----|--------|
| 0 | New Protection Domain |
| 1 | New Object Space (`SPC_OBJ`) |
| 2 | New Host Space (`SPC_HST`) |
| 3 | New Guest Space (`SPC_GST`) |
| 4 | New DMA Space (`SPC_DMA`) |
| 5 | New PIO Space (`SPC_PIO`) |
| 6 | New MSR Space (`SPC_MSR`) |

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `BAD_PAR`, `BAD_FTR`, `MEM_OBJ`, `MEM_CAP`

#### 5.3.2 Create Execution Context

```c
status = create_ec(SEL_OBJ sel,   // Created EC
                   SEL_OBJ pd,    // Owner PD
                   SEL_HST hvp,   // Host-Virtual Page Number
                   SEL_EVT evt,   // Event Selector Base
                   UINT16 cpu,    // CPU Number
                   UINTPTR sp)    // Initial Stack Pointer
```

**Flags:** `[ 3: 0 ][ 2: F ][ 1: T ][ 0: G ]`

- **G=0**: Host Execution Context (`EC_HST`)
  - T=0: Local Thread; T=1: Global Thread
- **G=1**: Guest Execution Context (`EC_GST`) — Virtual CPU
  - T=0: No time adjustment; T=1: Time offsetting
- **F**: FPU instructions enabled if F=1.

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `BAD_CPU`, `BAD_FTR`, `BAD_PAR`, `MEM_OBJ`, `MEM_CAP`

#### 5.3.3 Create Scheduling Context

```c
status = create_sc(SEL_OBJ sel,  // Created SC
                   SEL_OBJ pd,   // Owner PD
                   SEL_OBJ ec,   // Bound EC
                   SCD scd)      // Scheduling Context Descriptor
```

**Flags:** `[ 3..0: 0 ]`

**Prior to the hypercall:**
- `SPC_OBJ_CURRENT[ec]` must refer to a `CAP_OBJ_EC` with permission `BINDSC`.
- The owner PD and the owner PD of the bound EC must be the same.

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `BAD_PAR`, `MEM_OBJ`, `MEM_CAP`

#### 5.3.4 Create Portal

```c
status = create_pt(SEL_OBJ sel,  // Created PT
                   SEL_OBJ pd,   // Owner PD
                   SEL_OBJ ec,   // Bound EC
                   UINTPTR ip)   // Instruction Pointer
```

**Flags:** `[ 3..0: 0 ]`

- The created PT is bound to the EC with initial MTD = 0 and initial PID = 0.

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `MEM_OBJ`, `MEM_CAP`

#### 5.3.5 Create Semaphore

```c
status = create_sm(SEL_OBJ sel,  // Created SM
                   SEL_OBJ pd,   // Owner PD
                   UINT64 val)   // Initial Counter Value / ISD
```

**Flags:** `[ 3..1: 0 ][ 0: I ]`

- **I=0**: Regular Semaphore with initial counter value `val`.
- **I=1**: Interrupt Semaphore; `SPC_OBJ_CURRENT[pd]` must have permission `DC`; ISD must be valid; initial counter = 0.

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `BAD_PAR` (if I=1), `MEM_OBJ`, `MEM_CAP`

#### 5.3.6 Create Device Context

```c
status = create_dc(SEL_OBJ sel,  // Created DC
                   SEL_OBJ pd,   // Owner PD
                   DTD dtd)      // Device Topology Descriptor
```

**Flags:** `[ 3..0: 0 ]`

**Status codes:** `SUCCESS`, `ABORTED`, `BAD_CAP`, `MEM_OBJ`, `MEM_CAP`

---

### 5.4 Object Control

#### 5.4.1 Control Protection Domain

```c
status = ctrl_pd(SEL_OBJ src,  // SRC Space
                 SEL_OBJ dst,  // DST Space
                 SEL ssb,      // SRC Selector Base
                 SEL dsb,      // DST Selector Base
                 UINT5 ord,    // Order
                 UINT6 pmm,    // Permission Mask
                 MAD mad)      // Memory Attribute Descriptor
```

**Flags:** `[ 3..0: 0 ]`

**Description:** Delegates capabilities from the specified selector range in the source space to the destination space, optionally reducing permissions.

Capability delegation rules:
1. Capabilities are delegated from source range `src[ssb, ssb+2^ord)`.
2. Capabilities are delegated to destination range `dst[dsb, dsb+2^ord)`.
3. Destination capability permissions = source permissions AND `pmm`.
4. The destination capability is always `CAP0` if: the source is `CAP0`, the destination obtains zero permissions, or the source object is being destructed concurrently.

**Prior to the hypercall:**
- `SPC_OBJ_CURRENT[src]` must have permission `TAKE`.
- `SPC_OBJ_CURRENT[dst]` must have permission `GRANT`.
- `ssb` and `dsb` must be order-aligned.
- `ssb` and `dsb` must be equal if `src` refers to a PIO or MSR Space.

**Status codes:** `SUCCESS`, `BAD_CAP`, `BAD_PAR`, `MEM_CAP`

#### 5.4.2 Control Execution Context

```c
status = ctrl_ec(SEL_OBJ ec)  // Execution Context
```

**Flags:** `[ 3..1: 0 ][ 0: S ]`

**Description:** Forces the EC to enter the microhypervisor and generate a recall exception prior to its next exit.

- **S=0** (Weak Recall): Returns as soon as the recall exception has been pended.
- **S=1** (Strong Recall): Returns as soon as the recall exception has been observed.

**Status codes:** `SUCCESS`, `BAD_CAP`

#### 5.4.3 Control Scheduling Context

```c
status = ctrl_sc(SEL_OBJ sc,    // Scheduling Context
                 UINT64 &stc)   // Total Consumed Execution Time (out)
```

**Flags:** `[ 3..0: 0 ]`

**Description:** Returns the total consumed execution time as a System Time Counter (STC) value.

**Status codes:** `SUCCESS`, `BAD_CAP`

#### 5.4.4 Control Portal

```c
status = ctrl_pt(SEL_OBJ pt,  // Portal
                 UINTPTR pid,  // Portal Identifier
                 MTD mtd)     // Message Transfer Descriptor
```

**Flags:** `[ 3..0: 0 ]`

**Description:** Sets the Portal Identifier (PID) and Message Transfer Descriptor (MTD) for the specified Portal. Subsequent portal traversals will use the new MTD and return the new PID.

**Status codes:** `SUCCESS`, `BAD_CAP`

#### 5.4.5 Control Semaphore

```c
status = ctrl_sm(SEL_OBJ sm,   // Semaphore
                 UINT64 stc)   // Absolute Timeout
```

**Flags:** `[ 3..2: 0 ][ 1: Z ][ 0: D ]`

**Description:**

- **D=0** (Semaphore Up): Releases one blocked EC, or increments the counter. Timeout value and Z-flag are ignored.
- **D=1** (Semaphore Down): Decrements the counter (Z=0) or sets it to zero (Z=1) if > 0, otherwise blocks `EC_CURRENT`. If `stc` is non-zero, unblocks with timeout status when STC reaches or exceeds the specified value.

Blocking and releasing of ECs uses the FIFO queueing discipline.

**Status codes:** `SUCCESS`, `TIMEOUT`, `ABORTED`, `OVRFLOW`, `BAD_CAP`, `BAD_CPU`

---

### 5.5 Platform Management

#### 5.5.1 Control Hardware

```c
status = ctrl_hw(UINT desc)  // Descriptor
```

**Flags:** `[ 3..2: – ][ 1..0: OP ]`

**Description:** Modifies the platform hardware configuration or power management state. Must be invoked from the Root Protection Domain (`PD_ROOT`).

**Operations:**

| OP | Operation |
|----|-----------|
| 0 | S-State Transition (ACPI sleep/reset) |
| 4 | QOS Configuration (enable/disable CDP) |
| 5 | CAT/CDP L3 Capacity Bitmask |
| 6 | CAT/CDP L2 Capacity Bitmask |
| 7 | MBA Delay |

**S-State Transition Descriptor (OP=0):**
```
Bits: [ 55..9: – ][ 8..3: B ][ 2: A (wait) ][ ... : S ]
```

| S | A | B | Type | Description |
|---|---|---|------|-------------|
| 0x0 | 0x0 | 0x0 | Reset | Platform Reset |
| 0x1 | \_S1[0] | \_S1[1] | Shallow | S1: Stop Grant |
| 0x2 | \_S2[0] | \_S2[1] | Shallow | S2: Power-On Suspend |
| 0x3 | \_S3[0] | \_S3[1] | Shallow | S3: Suspend to RAM |
| 0x4 | \_S4[0] | \_S4[1] | Deep | S4: Suspend to Disk |
| 0x5 | \_S5[0] | \_S5[1] | — | S5: Soft Off |

**Status codes:** `SUCCESS`, `BAD_HYP`, `BAD_FTR`, `BAD_PAR`, `ABORTED`

#### 5.5.2 Assign Device

```c
status = assign_dev(SEL_OBJ dc,   // DMA Source Device
                    SEL_OBJ dma)  // DMA Space
```

**Flags:** `[ 3..0: 0 ]`

**Description:** Assigns a DMA-capable device to the specified DMA Space. DMA transactions of that device will be managed by the SMMU configured in the Device Context.

**Status codes:** `SUCCESS`, `TIMEOUT`, `ABORTED`, `BAD_CAP`, `BAD_DEV`, `MEM_OBJ`

#### 5.5.3 Assign Interrupt

```c
status = assign_int(SEL_OBJ sm,        // Interrupt Semaphore (GSI)
                    SEL_OBJ dc,        // MSI Source Device
                    UINT4 cfg,         // Interrupt Configuration
                    UINT16 cpu,        // Destination CPU Number
                    UINT16 idx,        // Table Index
                    UINTPTR &msi_addr, // OUT: MSI Address
                    UINTPTR &msi_data) // OUT: MSI Data
```

**Flags:** `[ 3..1: 0 ][ 0: A ]`

**Description:** Attaches (A=1) or Detaches (A=0) an Interrupt Semaphore in a semaphore table slot and configures the associated GSI.

**Interrupt Configuration (cfg):**
```
Bits: [ 3: O(wnership) ][ 2: P(olarity) ][ 1: T(rigger) ][ 0: M(ask) ]
```

| Field | 0 | 1 |
|-------|---|---|
| Mask | Unmasked | Masked† |
| Trigger | Edge-Triggered | Level-Triggered† |
| Polarity | Active-High | Active-Low† |
| Ownership | Host-Owned | Guest-Owned‡ |

> † Only valid for GSI type PIN  
> ‡ Only valid for Arm architecture

**Status codes:** `SUCCESS`, `BAD_CPU`, `BAD_CAP`, `BAD_DEV`, `BAD_PAR`, `ABORTED`

---

## 6. Booting

### 6.1 NOVA Microhypervisor

#### 6.1.1 NOVA Image

The bootloader must place all loadable (`PT_LOAD`) program segments of the NOVA microhypervisor into physical memory according to the physical addresses (`p_paddr`) and memory sizes (`p_memsz`) defined in the ELF executable.

Example:
```
Elf file type is EXEC (Executable file)
Entry point 0x404000
Program Headers:
  Type   Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flags Align
  LOAD   0x0000e8 0x0000000000404000 0x0000000000404000 0x00091c 0x00091c RW    0x8
  LOAD   0x00191c 0xffffffff8000491c 0x000000000040491c 0x014d4c 0xffb6e4 RW    0x1000
```

The bootloader may shift all loadable program segments lower or higher in physical memory by applying a uniform offset, subject to constraints in the Multiboot2 relocatable header tag.

#### 6.1.2 NOVA Integrity Measurement

The Reference Integrity Measurement (RIM) is printed during `make install`. On platforms with DRTM support, NOVA initiates a measured launch of itself, extending a Launch Integrity Measurement (LIM) into PCR 17 of the TPM.

- **Sensitive to (LIM ≠ RIM):** Changes in the attestable region; bootloader passing command-line parameters.
- **Insensitive to (LIM = RIM):** Hardware platform and NOVA patching; physical memory load address; memory map and other software modules.

#### 6.1.3 NOVA Spaces

**NOVA Object Space (`SPC_OBJ_NOVA`):**

| `SEL_OBJ` | Capability Type | Capability Resource | Permissions |
|-----------|----------------|---------------------|-------------|
| 2^SBW_OBJ-1 | `CAP_OBJ_SM` | Console Semaphore | All defined |
| 2^SBW_OBJ-2 | `CAP_OBJ_OBJ` | NOVA Object Space | TAKE |
| 2^SBW_OBJ-3 | `CAP_OBJ_HST` | NOVA Host Space | TAKE |
| 2^SBW_OBJ-4 | `CAP_OBJ_PIO` | NOVA PIO Space† | TAKE |
| 2^SBW_OBJ-5 | `CAP_OBJ_MSR` | NOVA MSR Space† | TAKE |
| 2^SBW_OBJ-6 | `CAP_OBJ_OBJ` | Root Object Space | All defined |
| 2^SBW_OBJ-7 | `CAP_OBJ_HST` | Root Host Space | All defined |
| 2^SBW_OBJ-8 | `CAP_OBJ_PIO` | Root PIO Space† | All defined |
| 0…CPU_MAX | `CAP_OBJ_SC` | Idle Scheduling Contexts | CTRL |

**NOVA Host Space:** `SEL_HST N` refers to `CAP0` for memory protected by NOVA, or `CAP_MEM` for the 4 KiB page frame at physical address `N << 12` with all defined permissions otherwise.

**NOVA PIO/MSR Spaces:** Similarly, `CAP0` for protected ports/MSRs, or `CAP_PIO`/`CAP_MSR` otherwise.

---

### 6.2 Root Protection Domain

After initialization, NOVA creates the following initial kernel objects:

- **Root Protection Domain (`PD_ROOT`)** with Root Object Space, Root Host Space, and Root PIO Space†
- **Root Execution Context (`EC_ROOT`)** on `CPU_BSP`, bound to the root spaces, with `SEL_EVT = 0`
- **Root Scheduling Context (`SC_ROOT`)** on `CPU_BSP`, bound to `EC_ROOT`, with COS=0, highest priority, Budget=1000ms

#### 6.2.1 Root Image

The Root PD image must be a valid ELF executable satisfying:
- `p_filesz = p_memsz` for each loadable segment
- `p_vaddr ≡ LOAD_ADDR + p_offset (mod PAGE_SIZE)` for each loadable segment

#### 6.2.2 Root Integrity Measurement

On platforms with DRTM support, NOVA extends a LIM of the Root Protection Domain's attestable region into **PCR 19** of the TPM.

#### 6.2.3 Root Spaces

**Root Object Space initial capabilities:**

| `SEL_OBJ` | Capability Type | Capability Resource | Permissions |
|-----------|----------------|---------------------|-------------|
| 2^SBW_OBJ-1 | `CAP_OBJ_OBJ` | NOVA Object Space | TAKE |
| 2^SBW_OBJ-2 | `CAP_OBJ_OBJ` | Root Object Space | All defined |
| 2^SBW_OBJ-3 | `CAP_OBJ_PD` | Root Protection Domain | All defined |
| 2^SBW_OBJ-4 | `CAP_OBJ_EC` | Root Execution Context | All defined |
| 2^SBW_OBJ-5 | `CAP_OBJ_SC` | Root Scheduling Context | All defined |

All other `SEL_OBJ` in Root Object Space initially refer to `CAP0`.

**Root Host Space:** NOVA maps the Root PD ELF program segments, the Hypervisor Information Page (read-only, 4 KiB below end of user-accessible virtual memory), and the UTCB (4 KiB below HIP). All other `SEL_HST` initially refer to `CAP0`.

**Root PIO Space:** All `SEL_PIO` initially refer to `CAP0`.

---

### 6.3 Hypervisor Information Page

The **Hypervisor Information Page (HIP)** is mapped in the Host Space of the Root PD. The initial Stack Pointer of `EC_ROOT` points to the HIP.

```
+0x00  [ Signature ][ Checksum ][ Length ]
+0x08  [ NOVA Start Address                ]
+0x10  [ NOVA End Address                  ]
+0x18  [ MBUF Start Address                ]
+0x20  [ MBUF End Address                  ]
+0x28  [ ROOT Start Address                ]
+0x30  [ ROOT End Address                  ]
+0x38  [ ACPI RSDP Address                 ]
+0x40  [ Framebuffer Address               ]
+0x48  [ Framebuffer Size                  ]
+0x50  [ Framebuffer Pixel Format ][ Pitch ]
+0x58  [ Framebuffer Resolution X ][ Y     ]
+0x60  [ UEFI Memory Map Address           ]
+0x68  [ UEFI Map Size ][ Desc Size ][ Ver ]
+0x70  [ STC Frequency                     ]
+0x78  [ SBW_OBJ ][ SBW_HST ][ SBW_GST ][ SBW_DMA ][ SBW_PIO ][ SBW_MSR ]
+0x80  [ MCO_OBJ ][ MCO_HST ][ MCO_GST ][ MCO_DMA ][ MCO_PIO ][ MCO_MSR ]
+0x88  [ / ][ KID_MAX ][ CPU_MAX ][ CPU_BSP ]
+0x90  [ SEL_GST/NOVA ][ SEL_GST/ARCH ][ SEL_HST/NOVA ][ SEL_HST/ARCH ]
+0x98  [ Platform Features                 ]
+0xa0  [ Architecture-Dependent ...        ]
```

**Key fields:**

- **Signature** – `0x41564f4e` identifies NOVA.
- **Checksum** – 16-bit-wise addition of entire HIP = 0.
- **STC Frequency** – Frequency of the System Time Counter in Hz.
- **SBW_xxx** – Selector bit widths for each space type.
- **MCO_xxx** – Maximum contiguous order to avoid partial failures during capability updates.
- **CPU_BSP** – Bootstrap Processor number.
- **CPU_MAX** – Maximum CPU number (total CPUs = CPU_MAX + 1).
- **KID_MAX** – Maximum KID number.

**Framebuffer Pixel Format:**
```
Bits: [ 31..30: N-1 ][ 29..25: BH ][ 24..20: BL ][ 19..15: GH ][ 14..10: GL ][ 9..5: RH ][ 4..0: RL ]
```
- N-1: 0=1B, 1=2B, 2=3B, 3=4B per pixel.

---

# Part IV: Application Binary Interface

## 7. ABI aarch64

### 7.1 Boot State

#### 7.1.1 NOVA Microhypervisor

**Preconditions:**
- CPU must execute in EL2 (hypervisor mode) or EL3 (monitor mode).
- Paging (MMU) must be disabled or provide an identity (1:1) mapping.
- Interrupts must be disabled (`PSTATE.DAIF = 0b1111`).
- Microhypervisor image region must be clean to PoC.
- DMA activity targeting the microhypervisor region must be quiesced.

**Multiboot v2 Launch:**

| Register | Value |
|----------|-------|
| IP | Physical address of NOVA ELF entry point |
| X0 | Multiboot v2 magic value (0x36d76289) |
| X1 | Physical address of Multiboot v2 information structure |
| Other | / |

**Multiboot v1 Launch:**

| Register | Value |
|----------|-------|
| IP | Physical address of NOVA ELF entry point |
| X0 | Multiboot v1 magic value (0x2badb002) |
| X1 | Physical address of Multiboot v1 information structure |
| Other | / |

**Legacy Launch:**

| Register | Value |
|----------|-------|
| IP | Physical address of NOVA ELF entry point |
| X0 | Physical address of the Flattened Device Tree (FDT) |
| X1 | Physical address of the Root Protection Domain ELF image |
| Other | / |

#### 7.1.2 Root Protection Domain

| Register | Value |
|----------|-------|
| IP | HVA of the Root PD ELF image entry point |
| SP | HVA of the Hypervisor Information Page (HIP) |
| X0 | X0 at boot time |
| X1 | X1 at boot time |
| X2 | X2 at boot time |
| Other | / |

### 7.2 Memory Map

The Root PD can obtain a list of available/reserved physical memory regions via:
- UEFI memory map (on UEFI platforms)
- Flattened Device Tree (on FDT platforms)

### 7.3 Class Of Service

Class Of Service (COS) is currently not supported on aarch64.

### 7.4 Protected Resources

**Protected physical memory regions:**
- NOVA microhypervisor (conveyed via HIP)
- GITS, GICD, GICR, GICC, GICH devices (conveyed via ACPI MADT or FDT)
- SMMU devices (conveyed via ACPI IORT or FDT)
- Firmware runtime services (conveyed via UEFI memory map)

### 7.5 Event-Specific Capability Selectors

For exception/intercept delivery, the microhypervisor performs an implicit portal traversal. The destination portal selector = `SEL_EVT + exception/intercept number`. Must refer to a `CAP_OBJ_PT` with permission `EVENT` bound to an EC on the same CPU; otherwise the affected EC is killed.

#### 7.5.1 Architectural Events (aarch64)

| SEL_OBJ | Exception / Intercept |
|---------|----------------------|
| SELEVT + 0x00 | Unknown Reason |
| SELEVT + 0x01 | Trapped WFI or WFE |
| SELEVT + 0x03 | Trapped MCR or MRC |
| SELEVT + 0x04 | Trapped MCRR or MRRC |
| SELEVT + 0x06 | Trapped LDC or STC |
| SELEVT + 0x07 | SME, SVE, SIMD, FPU |
| SELEVT + 0x0d | Branch Target Exception |
| SELEVT + 0x0e | Illegal Execution State |
| SELEVT + 0x11 | SVC (from AArch32 State) |
| SELEVT + 0x12 | HVC (from AArch32 State) |
| SELEVT + 0x13 | SMC (from AArch32 State) |
| SELEVT + 0x15 | SVC (from AArch64 State)* |
| SELEVT + 0x16 | HVC (from AArch64 State) |
| SELEVT + 0x17 | SMC (from AArch64 State) |
| SELEVT + 0x18 | Trapped MSR or MRS |
| SELEVT + 0x19 | Trapped SVE |
| SELEVT + 0x1a | Trapped ERET |
| SELEVT + 0x1c | PAuth Instruction Failure |
| SELEVT + 0x1e | Granule Protection Exception |
| SELEVT + 0x20 | Instruction Abort (lower EL) |
| SELEVT + 0x22 | PC Alignment Fault |
| SELEVT + 0x24 | Data Abort (lower EL) |
| SELEVT + 0x26 | SP Alignment Fault |
| SELEVT + 0x27 | Memory Operation Exception |
| SELEVT + 0x2c | Trapped FPU (AArch64) |
| SELEVT + 0x2f | SError |
| SELEVT + 0x30 | Breakpoint (lower EL) |
| SELEVT + 0x32 | Software Step (lower EL) |
| SELEVT + 0x34 | Watchpoint (lower EL) |
| SELEVT + 0x38 | BKPT (AArch32) |
| SELEVT + 0x3a | Vector Catch (AArch32) |
| SELEVT + 0x3c | BRK (AArch64) |

> \* These events may be handled by the microhypervisor, in which case they will not cause portal traversals.

#### 7.5.2 Microhypervisor Events (aarch64)

| SEL_OBJ | Event |
|---------|-------|
| SELEVT + SEL_ARCH + 0x0 | Startup |
| SELEVT + SEL_ARCH + 0x1 | Recall |
| SELEVT + SEL_ARCH + 0x2 | Virtual Timer |

`SEL_ARCH = SEL_HST/ARCH` for host events; `SEL_ARCH = SEL_GST/ARCH` for guest events.

### 7.6 Architecture-Dependent Structures

#### 7.6.1 Platform Features (aarch64)

```
Bits: [ 3..1: – ][ 0: SMMU ]
```

- **SMMU** – If set, SMMU is active.

#### 7.6.2 Hypervisor Information Page (aarch64 extension)

```
Arch+0x00: [ SPINUM ][ ESPINUM ][ LPINUM ]
Arch+0x08: [ / ][ CTXNUM ][ SMGNUM ]
```

- **SPINUM** – Total number of Shared Peripheral Interrupts (SPIs).
- **ESPINUM** – Total number of Extended Shared Peripheral Interrupts (ESPIs).
- **LPINUM** – Total number of Locality-Specific Peripheral Interrupts (LPIs).
- **SMGNUM** – Total number of SMMUv2 Stream Mapping Groups (SMGs).
- **CTXNUM** – Total number of SMMUv2 Translation Contexts (CTXs).

#### 7.6.3 User Thread Control Block (aarch64)

```
Offset   Contents
+0x000   X0 (R0)          X1 (R1)
+0x010   X2 (R2)          X3 (R3)
+0x020   X4 (R4)          X5 (R5)
+0x030   X6 (R6)          X7 (R7)
+0x040   X8 (R8_usr)      X9 (R9_usr)
+0x050   X10 (R10_usr)    X11 (R11_usr)
+0x060   X12 (R12_usr)    X13 (SP_usr)
+0x070   X14 (LR_usr)     X15 (SP_hyp)
+0x080   X16 (LR_irq)     X17 (SP_irq)
+0x090   X18 (LR_svc)     X19 (SP_svc)
+0x0a0   X20 (LR_abt)     X21 (SP_abt)
+0x0b0   X22 (LR_und)     X23 (SP_und)
+0x0c0   X24 (R8_fiq)     X25 (R9_fiq)
+0x0d0   X26 (R10_fiq)    X27 (R11_fiq)
+0x0e0   X28 (R12_fiq)    X29 (SP_fiq)
+0x0f0   X30 (LR_fiq)     SP_EL0
+0x100   TPIDR_EL0        TPIDRRO_EL0
+0x110   [A32] SPSR_abt   SPSR_fiq   SPSR_irq   SPSR_und
+0x120   [A32] DACR       IFSR       HSTR
+0x130   [EL1] SP_EL1     TPIDR_EL1
+0x140   CONTEXTIDR_EL1   ELR_EL1
+0x150   SPSR_EL1         ESR_EL1
+0x160   FAR_EL1          AFSR0_EL1
+0x170   AFSR1_EL1        TTBR0_EL1
+0x180   TTBR1_EL1        TCR_EL1
+0x190   MAIR_EL1         AMAIR_EL1
+0x1a0   VBAR_EL1         SCTLR_EL1
+0x1b0   MDSCR_EL1
+0x1c0   [EL2] HCR_EL2    HCRX_EL2
+0x1d0   VPIDR_EL2        VMPIDR_EL2
+0x1e0   ELR_EL2          SPSR_EL2
+0x1f0   ESR_EL2          FAR_EL2
+0x200   HPFAR_EL2
+0x210   [TMR] CNTV_CVAL_EL0  CNTV_CTL_EL0
+0x220   CNTKCTL_EL1      CNTVOFF_EL2
+0x230   [GIC] LR0        LR1
...
+0x2a0   LR14             LR15
+0x2b0   AP0R0  AP0R1  AP0R2  AP0R3
+0x2c0   AP1R0  AP1R1  AP1R2  AP1R3
+0x2d0   ELRSR            VMCR
+0x2e0   SEL_GST
```

#### 7.6.4 Message Transfer Descriptor (aarch64)

```
Bits [31..30]: –
Bit  29:       SPACES
Bit  28:       GIC
Bits 27..26:   –
Bit  25:       TMR
Bits 24..20:   EL2 fields (HPFAR, ESR_FAR, ELR_SPSR, IDR, HCR)
Bits 19..13:   EL1 fields (MDSCR, SCTLR_VBAR, MAIR, TCR, TTBR, AFSR, ESR_FAR, ELR_SPSR, IDR, SP)
Bits 12..11:   –
Bit  10:       A32_DIH
Bit  9:        A32_SPSR
Bits 8..7:     –
Bit  6:        EL0_IDR
Bit  5:        EL0_SP
Bit  4:        FPR
Bit  3:        GPR
Bit  2:        ICI
Bit  1:        –
Bit  0:        POISON
```

#### 7.6.5 Memory Attribute Descriptor (aarch64)

```
Bits: [ 31..5: – ][ 4..3: SH ][ 2..0: CA ]
```

**CA (Cacheability):**

| Encoding | Description |
|----------|-------------|
| 0x0 | DEV – Device |
| 0x1 | DEV_E – Device, Early Ack |
| 0x2 | DEV_RE – Device, Early Ack, Reordering |
| 0x3 | DEV_GRE – Device, Early Ack, Reordering, Gathering |
| 0x5 | MEM_NC – Memory, Inner/Outer Non-Cacheable |
| 0x6 | MEM_WT – Memory, Inner/Outer Write-Through |
| 0x7 | MEM_WB – Memory, Inner/Outer Write-Back |

**SH (Shareability):**

| Encoding | Description |
|----------|-------------|
| 0x0 | NONE – Not Shareable |
| 0x2 | OUTER – Outer Shareable |
| 0x3 | INNER – Inner Shareable |

#### 7.6.6 Interrupt Semaphore Descriptor (aarch64)

```
Bits: [ 63..24: – ][ 23..0: GSI ]
```

**GSI ranges:**
- [32, 32+SPINUM) – Shared Peripheral Interrupt (SPI)
- [4096, 4096+ESPINUM) – Extended Shared Peripheral Interrupt (ESPI)
- [8192, 8192+LPINUM) – Locality-Specific Peripheral Interrupt (LPI)

#### 7.6.7 Device Topology Descriptor (aarch64)

**SMMUv3:**
```
Word #0: [ DID ][ SID ]
Word #1: [ SMMU[63:12] ][ – ]
Word #2: [ GITS[63:12] ][ – ]
```

**SMMUv2:**
```
Word #0: [ DID ][ SID Mask ][ SID ]
Word #1: [ SMMU[63:12] ][ – ][ SMG ]
Word #2: [ GITS[63:12] ][ – ][ CTX ]
```

### 7.7 Calling Convention (aarch64)

Hypercalls are invoked via `svc #0`. The hypercall identifier format:

```
Bits: [ 7..4: flags ][ 3..0: number ]
```

**IPC Call:**
```
 Input                              Output
 X0: pt[63-8] | hypercall[7-0]     X0: status[7-0]
 X1: mtd[31-0]                     X1: mtd[31-0]
 IP: svc #0                        IP: IP+4
```

**IPC Reply:**
```
 Input                              Output
 X0: hypercall[7-0]                 X0: pid
 X1: mtd[31-0]                     X1: mtd[31-0]
 IP: svc #0                        IP: Portal IP
```

**Create Execution Context:**
```
 Input                              Output
 X0: sel[63-8] | hypercall[7-0]    X0: status[7-0]
 X1: pd                            X1: ≡
 X2: hvp[63-12]                    X2: ≡
 X3: evt[63-16] | cpu[15-0]        X3: ≡
 X4: sp                            X4: ≡
 IP: svc #0                        IP: IP+4
```

**Assign Interrupt:**
```
 Input                              Output
 X0: sm[63-8] | hypercall[7-0]     X0: status[7-0]
 X1: dc                            X1: msi_addr
 X2: idx[47-32] | cpu[31-16] | cfg X2: msi_data
 IP: svc #0                        IP: IP+4
```

### 7.8 Supplementary Functionality (aarch64)

**Secure Monitor Call** (restricted to Root Protection Domain, via `svc #1`):

The microhypervisor proxies the following SMC service calls to platform firmware:

| Service | Description | Functions |
|---------|-------------|-----------|
| 0x2 | SIP Service Calls | 0x0000–0xffff |
| 0x4 | Standard Secure Service Calls | 0x0050–0x005f (TRNG), 0x0130–0x013f (PCI) |
| 0x30–0x31 | Trusted Application Calls | 0x0000–0xffff |
| 0x32–0x3f | Trusted OS Calls | 0x0000–0xffff |

---

## 8. ABI x86-64

### 8.1 Boot State

#### 8.1.1 NOVA Microhypervisor

**Preconditions:**
- CPU state must conform to a Multiboot Specification v2 or v1 machine state.
- DMA activity targeting the microhypervisor region must be quiesced (IOMMU-protected recommended).

**Multiboot v2 Launch:**

| Register | Value |
|----------|-------|
| EIP | Physical address of NOVA ELF entry point |
| EAX | Multiboot v2 magic value (0x36d76289) |
| EBX | Physical address of Multiboot v2 information structure |
| Other | / |

**Multiboot v1 Launch:**

| Register | Value |
|----------|-------|
| EIP | Physical address of NOVA ELF entry point |
| EAX | Multiboot v1 magic value (0x2badb002) |
| EBX | Physical address of Multiboot v1 information structure |
| Other | / |

#### 8.1.2 Root Protection Domain

| Register | Value |
|----------|-------|
| RIP | HVA of the Root PD ELF image entry point |
| RSP | HVA of the Hypervisor Information Page (HIP) |
| RDI | EAX at boot time |
| RSI | EBX at boot time |
| Other | / |

### 8.2 Memory Map

The Root PD can obtain a list of available/reserved physical memory regions via:
- UEFI memory map (Multiboot v2 with UEFI boot services)
- Multiboot v2 memory map
- Multiboot v1 memory map

### 8.3 Class Of Service (x86)

COS support is indicated by `CPUID leaf 0x7, sub-leaf 0x0: EBX[15]`. The Root PD must perform the following on each CPU:

1. Invoke `ctrl_hw` to establish a valid QOS configuration (enabling/disabling CDP).
2. Determine `COS_NUM` from CPUID leaves 0x10 sub-leaves 0x1–0x3.
3. Invoke `ctrl_hw` to configure L3/L2 Capacity Bitmasks and MBA Delay for each COS.

### 8.4 Protected Resources (x86)

**Protected physical memory:**
- NOVA microhypervisor (via HIP)
- LAPIC, IOAPIC devices (via ACPI MADT)
- IOMMU devices (via ACPI DMAR or IVRS)
- DPR and TXT memory (except TPM localities 0 and 1)
- MSI range at 0xfee00000
- Firmware runtime services (via UEFI memory map)

**Protected I/O Ports:**

| I/O Port | CAP_PIO | VMM Handling |
|----------|---------|--------------|
| PM1a_CNT, PM1b_CNT, PM2_CNT, SMI_CMD | CAP0 (always) | Emulate |
| Other | A (if A not set) | Emulate or Passthrough |

**Protected MSRs (key entries):**

| MSR | CAP_MSR | RDMSR Exit | WRMSR Exit |
|-----|---------|------------|------------|
| IA32_SYSENTER_CS/ESP/EIP* | RW | If R not set | If W not set |
| IA32_PAT*, IA32_EFER* | RW | If R not set | If W not set |
| IA32_FS/GS/KERNEL_GS_BASE* | RW | If R not set | If W not set |
| IA32_PRED_CMD, IA32_FLUSH_CMD | W | Always | If W not set |
| IA32_TSC, IA32_PLATFORM_ID | R | If R not set | Always |
| Other | CAP0 | Always | — |

> \* The VMM can read/write the guest-effective MSR value via the UTCB.

### 8.5 Event-Specific Capability Selectors (x86)

#### 8.5.1 Architectural Events

**Host Exceptions:**

| SEL_OBJ | Exception |
|---------|-----------|
| SELEVT + 0x00 | #DE (Divide Error) |
| SELEVT + 0x01 | #DB (Debug) |
| SELEVT + 0x03 | #BP (Breakpoint) |
| SELEVT + 0x04 | #OF (Overflow) |
| SELEVT + 0x05 | #BR (Bound Range) |
| SELEVT + 0x06 | #UD (Invalid Opcode) |
| SELEVT + 0x07 | #NM* |
| SELEVT + 0x08 | #DF* |
| SELEVT + 0x0a | #TS* |
| SELEVT + 0x0b | #NP |
| SELEVT + 0x0c | #SS |
| SELEVT + 0x0d | #GP |
| SELEVT + 0x0e | #PF |
| SELEVT + 0x10 | #MF |
| SELEVT + 0x11 | #AC |
| SELEVT + 0x12 | #MC* |
| SELEVT + 0x13 | #XM |
| SELEVT + 0x14 | #VE |
| SELEVT + 0x15 | #CP |

**Guest Intercepts (VMX) — key entries:**

| SEL_OBJ | Intercept |
|---------|-----------|
| SELEVT + 0x00 | Exception or NMI* |
| SELEVT + 0x01 | External Interrupt* |
| SELEVT + 0x02 | Triple Fault† |
| SELEVT + 0x0a | CPUID† |
| SELEVT + 0x0c | HLT† |
| SELEVT + 0x0d | INVD† |
| SELEVT + 0x1c | CR Access* |
| SELEVT + 0x1e | I/O Access† |
| SELEVT + 0x1f | RDMSR† |
| SELEVT + 0x20 | WRMSR† |
| SELEVT + 0x30 | EPT Violation† |
| SELEVT + 0x31 | EPT Misconfiguration |
| SELEVT + 0x4a | Bus Lock |
| SELEVT + 0x4b | Instruction Timeout |

*(See full specification for complete list of 0x00–0x5f intercept selectors)*

#### 8.5.2 Microhypervisor Events (x86)

| SEL_OBJ | Event |
|---------|-------|
| SELEVT + SEL_ARCH + 0x0 | Startup |
| SELEVT + SEL_ARCH + 0x1 | Recall |

### 8.6 Architecture-Dependent Structures (x86)

#### 8.6.1 Platform Features (x86)

```
Bits: [ 3..2: – ][ 1: SVM ][ 0: VMX ][ ... : IOMMU ]
Bits: [ 3: – ][ 2: SVM ][ 1: VMX ][ 0: IOMMU ]
```

- **IOMMU** – If set, IOMMU is active.
- **VMX** – If set, Intel VMX is active.
- **SVM** – If set, AMD SVM is active.

#### 8.6.2 Hypervisor Information Page (x86 extension)

```
Arch+0x00: [ VECNUM ][ PINNUM ][ / ][ GSINUM ]
Arch+0x08: [ Event Log Physical Address    ]
Arch+0x10: [ Event Log Offset ][ Event Log Size ]
```

- **VECNUM** – Total number of per-CPU interrupt vectors.
- **PINNUM** – Total number of PIN-type GSIs (GSI 0 through PINNUM-1 are PIN; others are MSI).
- **GSINUM** – Total number of Global System Interrupts.
- **Event Log** – TPM event log address, size, and current offset.

#### 8.6.3 User Thread Control Block (x86)

```
Offset   Contents
+0x000   R0 (RAX)         R1 (RCX)
+0x010   R2 (RDX)         R3 (RBX)
+0x020   R4 (RSP)         R5 (RBP)
+0x030   R6 (RSI)         R7 (RDI)
+0x040   R8               R9
+0x050   R10              R11
+0x060   R12              R13
+0x070   R14              R15
+0x080   RFLAGS           RIP
+0x090   Instruction Length / Instruction Info / Interruptibility / Activity
+0x0a0   1st Qualification  2nd Qualification
+0x0b0   3rd Qualification
+0x0c0   1st Exec Controls  2nd Exec Controls  3rd Exec Controls
+0x0d0   CR0 Intercepts   CR4 Intercepts
+0x0e0   EXC Intercepts   PF Error Mask  PF Error Match  TPR Threshold
+0x0f0   Interruption Info / Interruption Error / IDT Vectoring Info / IDT Vectoring Error
+0x100   [CS] SEL  AR*  Limit  Base
+0x110   [SS] SEL  AR*  Limit  Base
+0x120   [DS] SEL  AR*  Limit  Base
+0x130   [ES] SEL  AR*  Limit  Base
+0x140   [FS] SEL  AR*  Limit  Base
+0x150   [GS] SEL  AR*  Limit  Base
+0x160   [TR] SEL  AR*  Limit  Base
+0x170   [LDTR] SEL  AR*  Limit  Base
+0x180   [GDTR] Limit  Base
+0x190   [IDTR] Limit  Base
+0x1a0   PDPTE0           PDPTE1
+0x1b0   PDPTE2           PDPTE3
+0x1c0   CR0              CR2
+0x1d0   CR3              CR4
+0x1e0   CR8              DR7
+0x1f0   XCR0             IA32_XSS
+0x200   IA32_SGXLEPUBKEYHASH0  IA32_SGXLEPUBKEYHASH1
+0x210   IA32_SGXLEPUBKEYHASH2  IA32_SGXLEPUBKEYHASH3
+0x220   IA32_APIC_BASE   IA32_SYSENTER_CS
+0x230   IA32_SYSENTER_ESP  IA32_SYSENTER_EIP
+0x240   IA32_PAT         IA32_EFER
+0x250   IA32_STAR        IA32_LSTAR
+0x260   IA32_FMASK       IA32_KERNEL_GS_BASE
+0x270   IA32_TSC_AUX     SEL_GST
+0x280   SEL_PIO          SEL_MSR
```

> \* See Section 8.6.3.1 for Segment Access Rights encoding.

**Segment Access Rights Encoding:**
```
Bits: [ 11: U(nusable) ][ 10: – ][ 9: G(ranularity) ][ 8: D/B ]
      [ 7: L ][ 6: AVL ][ 5: P(resent) ][ 4..3: DPL ][ 2: S ][ 1..0: Type ]
```

**Interruption Information Encoding:**
```
Bits: [ 31: V(alid) ][ 30..13: – ][ 12: N(MI window) ][ 11: I(nterrupt window) ]
      [ 10: E(rror code) ][ 9..8: – ][ 7..4: Type ][ 3..0: Vector ]
```

#### 8.6.4 Message Transfer Descriptor (x86)

```
Bit  30:     SPACES
Bit  29:     FPU
Bit  28:     TLB
Bit  27:     TSC
Bit  26:     KERNEL_GS
Bit  25:     EFER
Bit  24:     PAT
Bit  23:     SYSENTER
Bit  22:     SYSCALL
Bit  21:     APIC
Bit  20:     SGX
Bit  19:     XSAVE
Bit  18:     DR
Bit  17:     CR
Bit  16:     PDPTE
Bit  15:     IDTR
Bit  14:     GDTR
Bit  13:     LDTR
Bit  12:     TR
Bit  11:     FS/GS
Bit  10:     DS/ES
Bit   9:     CS/SS
Bit   8:     INJ
Bit   7:     TPR
Bit   6:     CTRL
Bit   5:     QUAL
Bit   4:     STA
Bit   3:     RIP
Bit   2:     RFLAGS
Bit   1:     GPR8–15
Bit   0:     GPR0–7 / POISON
```

#### 8.6.5 Memory Attribute Descriptor (x86)

```
Bits: [ 31..18: – ][ 17..3: KID ][ 2..0: CA ]
```

**CA (Cacheability):**

| Encoding | Description |
|----------|-------------|
| 0x0 | WB – Write Back |
| 0x1 | WT – Write Through |
| 0x2 | WC – Write Combining |
| 0x3 | UC – Strong Uncacheable |
| 0x4 | WP – Write Protected |

- **KID** – Key identifier for memory encryption; must be ≤ KID_MAX.

#### 8.6.6 Interrupt Semaphore Descriptor (x86)

```
Bits: [ 63..16: – ][ 15: S(egment) ][ 14..0: GSI ]
```

- **GSI** – Designates the interrupt in range [0, GSINUM).

#### 8.6.7 Device Topology Descriptor (x86)

```
Word #0: [ – ][ S ][ B ][ D ][ F ]
Word #1: [ SMMU[63:12] ][ – ]
Word #2: [ – ]
```

- **S, B, D, F** – PCI Segment/Bus/Device/Function (SBDF).
- **SMMU** – Host-Physical Address of the SMMU for DMA and MSI translation.

### 8.7 Calling Convention (x86-64)

Hypercalls are invoked via the `syscall` instruction. The hypercall identifier format:

```
Bits: [ 7..4: flags ][ 3..0: number ]
```

**IPC Call:**
```
 Input                              Output
 RDI: pt[63-8] | hypercall[7-0]    RDI: status[7-0]
 RSI: mtd[31-0]                    RSI: mtd[31-0]
 RCX: –                            RCX: RIP+2
 R11: –                            R11: 0x202
 RIP: syscall                      RIP: RIP+2
```

**IPC Reply:**
```
 Input                              Output
 RDI: hypercall[7-0]                RDI: pid
 RSI: mtd[31-0]                    RSI: mtd[31-0]
 RIP: syscall                      RIP: Portal IP
```

**Create Execution Context:**
```
 Input                              Output
 RDI: sel[63-8] | hypercall[7-0]   RDI: status[7-0]
 RSI: pd                           RSI: ≡
 RDX: hvp[63-12]                   RDX: ≡
 RAX: evt[63-16] | cpu[15-0]       RAX: ≡
 R8:  sp                           R8:  ≡
```

**Assign Interrupt:**
```
 Input                              Output
 RDI: sm[63-8] | hypercall[7-0]    RDI: status[7-0]
 RSI: dc                           RSI: msi_addr
 RDX: idx[47-32] | cpu[31-16] | cfg RDX: msi_data
```

*(Other hypercalls follow similar conventions — see aarch64 for structure reference, replacing X registers with RDI/RSI/RDX/RAX/R8)*

---

# Part V: Appendix

## A. Acronyms

| Acronym | Expansion |
|---------|-----------|
| ACPI | Advanced Configuration and Power Interface |
| BSP | Bootstrap Processor |
| CAP | Capability |
| CAT | Cache Allocation Technology |
| CDP | Code and Data Prioritization |
| COS | Class Of Service |
| CPU | Central Processing Unit |
| CRTM | Core Root of Trust for Measurement |
| DC | Device Context |
| DMA | Direct Memory Access |
| DRTM | Dynamic Root of Trust for Measurement |
| DTD | Device Topology Descriptor |
| DVA | DMA-Virtual Address |
| EC | Execution Context |
| ELF | Executable and Linkable Format |
| ESPI | Extended Shared Peripheral Interrupt |
| FDT | Flattened Device Tree |
| FPU | Floating Point Unit |
| GIC | Generic Interrupt Controller |
| GPA | Guest-Physical Address |
| GSI | Global System Interrupt |
| HIP | Hypervisor Information Page |
| HPA | Host-Physical Address |
| HVA | Host-Virtual Address |
| IOMMU | I/O Memory Management Unit |
| IPC | Inter-Process Communication |
| ISD | Interrupt Semaphore Descriptor |
| KID | Key Identifier |
| LIM | Launch Integrity Measurement |
| LPI | Locality-Specific Peripheral Interrupt |
| MAD | Memory Attribute Descriptor |
| MBA | Memory Bandwidth Allocation |
| MSI | Message-Signaled Interrupt |
| MSR | Model-Specific Register |
| MTD | Message Transfer Descriptor |
| NOVA | NOVA OS Virtualization Architecture |
| PCI | Peripheral Component Interconnect |
| PCR | Platform Configuration Register |
| PD | Protection Domain |
| PID | Portal Identifier |
| PIO | Port-Based I/O |
| PT | Portal |
| QOS | Quality Of Service |
| RIM | Reference Integrity Measurement |
| SBDF | PCI Segment/Bus/Device/Function |
| SC | Scheduling Context |
| SCD | Scheduling Context Descriptor |
| SEL | Capability Selector |
| SID | Stream Identifier |
| SM | Semaphore |
| SMG | Stream Mapping Group |
| SMMU | System Memory Management Unit |
| SP | Stack Pointer |
| SPI | Shared Peripheral Interrupt |
| STC | System Time Counter |
| TPM | Trusted Platform Module |
| TXT | Trusted Execution Technology |
| UART | Universal Asynchronous Receiver Transmitter |
| UEFI | Unified Extensible Firmware Interface |
| UTCB | User Thread Control Block |
| VMM | Virtual-Machine Monitor |

## B. Bibliography

1. RFC 2119 – Internet Engineering Task Force (IETF), 1997. https://tools.ietf.org/html/rfc2119
2. Udo Steinberg and Bernhard Kauer. *NOVA: A Microhypervisor-Based Secure Virtualization Architecture.* EuroSys 2010.
3. *Arm Architecture Reference Manual ARMv8.* Arm Limited, 2025. DDI0487L.b.
4. *Intel 64 and IA-32 Architectures Software Developer's Manual.* Intel Corporation, 2025. Document 325462-088.
5. *AMD64 Architecture Programmer's Manual: Volumes 1–5.* AMD, 2024. Document 40332.
6. *The Multiboot2 Specification.* Version 2.0, 2016.
7. *Advanced Configuration and Power Interface (ACPI) Specification.* UEFI Forum, Version 6.6, 2025.
8. *Unified Extensible Firmware Interface (UEFI) Specification.* UEFI Forum, Version 2.11, 2024.
9. *The Multiboot Specification.* Version 0.6.96, 2010.
10. *Arm Generic Interrupt Controller Architecture Specification Version 2.* IHI0048B.b.
11. *Arm Generic Interrupt Controller Architecture Specification Version 3 and 4.* IHI0069H.b.
12. *Arm System Memory Management Unit Architecture Specification Version 2.* IHI0062D.c.
13. *Arm System Memory Management Unit Architecture Specification Version 3.* IHI0070G.b.
14. *Arm SMC Calling Convention.* DEN0028G, 2025.
15. *Arm True Random Number Generator Firmware Interface.* DEN0098, 2022.
16. *Arm PCI Configuration Space Access Firmware Interface.* DEN0115, 2022.
17. *Intel Virtualization Technology for Directed I/O Architecture Specification.* D51397-017, Rev 5.0, 2024.
18. *AMD I/O Virtualization Technology (IOMMU) Specification.* Document 48882, 2023.
19. *Intel Trusted Execution Technology (Intel TXT) Software Development Guide.* 315168-017, Rev 17.6, 2025.
20. *TCG PC Client Platform, Firmware Profile Specification.* Version 1.06, Rev 52, 2023.
21. *TCG PC Client Platform, TPM Profile Specification for TPM 2.0.* Version 1.05, Rev 14, 2020.
22. *TCG Trusted Platform Module Library Specification, Family "2.0".* Rev 1.83, 2024.
23. *ELF Object File Format.* Xinuos, Version 4.3, 2025.
24. *Devicetree Specification.* Linaro Limited, Version 0.4, 2023.
25. *PCI Local Bus Specification.* PCI-SIG, Revision 3.0, 2004.
26. *PCI Express Base Specification.* PCI-SIG, Revision 7.0, 2025.

---

## C. Console

### C.1 Memory-Buffer Console

The NOVA microhypervisor implements a memory-buffer console for boot-time and run-time debug output.

**Structure:**
```
Start+0x00: [ RdIdx ][ WrIdx ]   (Header)
Start+0x08: [ Char0  Char1  Char2  ... Char7  ]
Start+0x10: [ Char8  Char9  ... Char15        ]
...
End:        [ Char(N-2) Char(N-1)             ]
```

Buffer size: `N = MBUF_End_Address - MBUF_Start_Address - header_size`

**Operation:**
- **RdIdx** (0 to N-1): Points to the next character the consumer will read.
- **WrIdx** (0 to N-1): Points to the next character NOVA will write (advanced only by NOVA).
- Buffer is empty when RdIdx = WrIdx.
- When full, NOVA advances RdIdx (discards oldest character).
- At the end of each line, NOVA invokes `ctrl_sm` (Up) on the signaling semaphore.
- Consumers should use `ctrl_sm` (Down) instead of polling WrIdx.

> The memory-buffer console is always enabled. The Root PD can delegate the data structure to any Protection Domain to act as console consumer.

### C.2 Framebuffer Console

Requires a UEFI platform with Graphics Output Protocol and an EDID-capable display. Provides boot-time-only debug output until hardware ownership transitions to the Root PD.

> Can be disabled via the `nofbuf` command-line parameter.

### C.3 UART Consoles

Requires an RS232 port/header connected via serial cable. Provides boot-time-only debug output. Receiver terminal must be configured for **115200 baud, 8N1 mode**.

> Can be disabled via the `nouart` command-line parameter. Disabling UART consoles can significantly improve boot time.

---

## D. Download

The source code of the NOVA microhypervisor and the latest version of this document can be downloaded from GitHub:

**https://github.com/udosteinberg/NOVA**
