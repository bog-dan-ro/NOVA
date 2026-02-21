# NOVA Microhypervisor Interface Specification

_Converted automatically from PDF to Markdown._

```
NOVA Microhypervisor
Interface Specification
Udo Steinberg
udo@hypervisor.org
December 1, 2025

Copyright ©2006–2011 Udo Steinberg, Technische Universität Dresden
Copyright ©2012–2013 Udo Steinberg, Intel Corporation
Copyright ©2014–2016 Udo Steinberg, FireEye, Inc.
Copyright ©2019–2025 Udo Steinberg, BlueRock Security, Inc.
This specification is provided "as is" and may contain defects or deficiencies which cannot or will not be
corrected. The author makes no representations or warranties, either expressed or implied, including but not
limited to, warranties of merchantability, fitness for a particular purpose, or non-infringement that the contents
of the specification are suitable for any purpose or that any practice or implementation of such contents will not
infringe any third party patents, copyrights, trade secrets or other rights.
The specification could include technical inaccuracies or typographical errors. Additions and changes are
periodically made to the information therein; these will be incorporated into new versions of the specification,
if any.

Contents
I Introduction 1
1 System Architecture 2
II Basic Abstractions 3
2 Kernel Objects 4
2.1 Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4
2.1.1 Object Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4
2.1.2 Host Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4
2.1.3 Guest Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 4
2.1.4 DMA Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
2.1.5 PIO Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
2.1.6 MSR Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 5
2.2 Execution Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
2.2.1 Host Execution Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
2.2.2 Guest Execution Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 6
2.3 Scheduling Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
2.4 Portal . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
2.5 Semaphore . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
2.6 Device Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 7
3 Hardware Resources 8
3.1 System Time Counter . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 8
III Application Programming Interface 9
4 Data Types 10
4.1 Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.1 Null Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.2 Object Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.2.1 Object Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.2.2 Host Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.2.3 Guest Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 10
4.1.2.4 DMA Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4.1.2.5 PIO Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4.1.2.6 MSR Space Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4.1.2.7 Protection Domain Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4.1.2.8 Execution Context Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . 11
4.1.2.9 Scheduling Context Capability . . . . . . . . . . . . . . . . . . . . . . . . . . 12
4.1.2.10 Portal Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
4.1.2.11 Semaphore Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
4.1.2.12 Device Context Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
4.1.3 Memory Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 12
4.1.4 PIO Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13
4.1.5 MSR Capability . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 13
4.2 Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.2.1 Object Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
i

4.2.2 Host Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.2.3 Guest Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.2.4 DMA Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.2.5 PIO Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.2.6 MSR Capability Selector . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 14
4.3 User Thread Control Block . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
4.3.1 Regular Layout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
4.3.2 Architectural Layout . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 15
4.4 Message Transfer Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
4.4.1 Regular IPC . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
4.4.2 Architectural IPC . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 16
4.5 Scheduling Context Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 17
5 Hypercalls 18
5.1 Definitions . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18
5.1.1 Hypercall Numbers . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18
5.1.2 Status Codes . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 18
5.2 Communication . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19
5.2.1 IPC Call . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 19
5.2.2 IPC Reply . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 20
5.3 Object Creation . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
5.3.1 Create Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 21
5.3.2 Create Execution Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 23
5.3.3 Create Scheduling Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 25
5.3.4 Create Portal . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 26
5.3.5 Create Semaphore . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 27
5.3.6 Create Device Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 29
5.4 Object Control . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
5.4.1 Control Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 30
5.4.2 Control Execution Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 32
5.4.3 Control Scheduling Context . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 33
5.4.4 Control Portal . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 34
5.4.5 Control Semaphore . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 35
5.5 Platform Management . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 36
5.5.1 Control Hardware . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 36
5.5.2 Assign Device . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 39
5.5.3 Assign Interrupt . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 40
6 Booting 42
6.1 NOV A Microhypervisor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 42
6.1.1 NOV A Image . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 42
6.1.2 NOV A Integrity Measurement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 42
6.1.3 NOV A Spaces . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43
6.1.3.1 NOV A Object Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43
6.1.3.2 NOV A Host Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43
6.1.3.3 NOV A PIO Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43
6.1.3.4 NOV A MSR Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 43
6.2 Root Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 44
6.2.1 Root Image . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 44
6.2.2 Root Integrity Measurement . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 45
6.2.3 Root Spaces . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 45
6.2.3.1 Root Object Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 45
6.2.3.2 Root Host Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 45
6.2.3.3 Root PIO Space . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 45
6.3 Hypervisor Information Page . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 46
ii

IV Application Binary Interface 49
7 ABI aarch64 50
7.1 Boot State . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 50
7.1.1 NOV A Microhypervisor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 50
7.1.1.1 Multiboot v2 Launch . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 50
7.1.1.2 Multiboot v1 Launch . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 50
7.1.1.3 Legacy Launch . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 50
7.1.2 Root Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 51
7.2 Memory Map . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 51
7.3 Class Of Service . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 51
7.4 Protected Resources . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 51
7.4.1 Physical Memory . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 51
7.5 Event-Specific Capability Selectors . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 52
7.5.1 Architectural Events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 52
7.5.2 Microhypervisor Events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 53
7.6 Architecture-Dependent Structures . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 54
7.6.1 Platform Features . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 54
7.6.2 Hypervisor Information Page . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 54
7.6.3 User Thread Control Block . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 55
7.6.4 Message Transfer Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 56
7.6.5 Memory Attribute Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 57
7.6.6 Interrupt Semaphore Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 57
7.6.7 Device Topology Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 58
7.6.7.1 SMMUv3 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 58
7.6.7.2 SMMUv2 . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 58
7.7 Calling Convention . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 59
7.8 Supplementary Functionality . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 62
8 ABI x86-64 63
8.1 Boot State . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63
8.1.1 NOV A Microhypervisor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63
8.1.1.1 Multiboot v2 Launch . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63
8.1.1.2 Multiboot v1 Launch . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63
8.1.2 Root Protection Domain . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63
8.2 Memory Map . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 64
8.3 Class Of Service . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 64
8.4 Protected Resources . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 64
8.4.1 Physical Memory . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 64
8.4.2 I /O Ports . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 65
8.4.3 Model-Specific Registers . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 65
8.5 Event-Specific Capability Selectors . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 66
8.5.1 Architectural Events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 66
8.5.2 Microhypervisor Events . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 68
8.6 Architecture-Dependent Structures . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 69
8.6.1 Platform Features . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 69
8.6.2 Hypervisor Information Page . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 69
8.6.3 User Thread Control Block . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 70
8.6.3.1 Encoding: Segment Access Rights . . . . . . . . . . . . . . . . . . . . . . . . 71
8.6.3.2 Encoding: Interruption Information . . . . . . . . . . . . . . . . . . . . . . . . 71
8.6.4 Message Transfer Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 72
8.6.5 Memory Attribute Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 73
8.6.6 Interrupt Semaphore Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 73
8.6.7 Device Topology Descriptor . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 73
8.7 Calling Convention . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 74
iii

V Appendix 78
A Acronyms 79
B Bibliography 83
C Console 85
C.1 Memory-Bu ffer Console . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 85
C.2 Framebu ffer Console . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 86
C.3 UART Consoles . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 86
D Download 87
iv

Notation
The key words must ,must not ,required ,should ,should not ,recommended ,may andoptional in this document
are to be interpreted as described in RFC 2119 [1].
Throughout this document, the following symbols are used:
∼ Indicates that the value of this parameter or field is unknown . The microhypervisor cannot ensure that the
value does not leak information across protection domain boundaries.
/Indicates that the value of this parameter or field is undefined . The microhypervisor ensures that the value
does not leak information across protection domain boundaries. Future versions of this specification may
define a value for the parameter or field.
– Indicates that the value of this parameter or field is ignored . Future versions of this specification may define
a meaning for the parameter or field.
≡Indicates that the value of this parameter or field is unchanged . The microhypervisor preserves the value
across hypercalls.
v

Part I
Introduction
1

1 System Architecture
The NOV A OSVirtualization Architecture [2] (NOV A) facilitates the coexistence of multiple legacy guest
operating systems and a user-mode host framework on a single platform. The core system leverages hardware
virtualization technology provided by modern x86 or Arm platforms and comprises the NOV A microhypervisor
and one or more Virtual-Machine Monitors (VMMs).
host
Root Partition Manager Drivers ApplicationsVMMVM VM VM
Guest
Operating
System
userVMM VMM VMM
MicrohypervisorguestApplianceVirtualVM
kernel... ...
... ...UnikernelContainer
Runtime
Figure 1.1: System Architecture
Figure 1.1 shows the structure of the system. The microhypervisor is the only component executing in privileged
host/kernel mode. It isolates the various user-mode components, including the virtual-machine monitors, from
one another by placing them in di fferent protection domains in unprivileged host /user mode. Each legacy guest
operating system runs in its own virtual-machine environment in guest mode and is therefore isolated from the
other components.
Besides spatial and temporal isolation, the microhypervisor also provides mechanisms for partitioning and
delegation of platform resources, such as CPU time, physical memory, I /O ports and hardware interrupts and
for establishing communication channels and signaling between di fferent protection domains.
The virtual-machine monitors handle virtualization events and implement virtual devices that enable legacy
guest operating systems to function in the same manner as they would on bare-metal hardware. Providing this
functionality outside the microhypervisor in the VMMs reduces the size of the trusted computing base significantly
for all components that do not require virtualization support.
The architecture and interfaces of the VMM and the user-mode host framework are not described in this document.
2

Part II
Basic Abstractions
3

2 Kernel Objects
2.1 Protection Domain
1. The Protection Domain (PD) is a unit of protection and spatial isolation.
2. Access to a Protection Domain is controlled by a PD Capability (CAP OBJ PD).
3. Di fferent types of spaces can be created for a PD – see Section 4.2 for details regarding availability and size.
4. Spaces store Capabilities (CAPs) for kernel objects or platform resources that Execution Contexts in that
Protection Domain can access.
The following subsections describe each space in more detail.
2.1.1 Object Space
1. Only one Object Space (SPC OBJ) can be created per Protection Domain.
2. Access to an Object Space is controlled by an Object Space Capability (CAP OBJ OBJ).
3. Each hypercall invoked by a Host Execution Context explicitly specifies an Object Capability Selector to
designate the kernel object on which it operates.
4. The Object Capability Selector (SEL OBJ) serves as index into the EC’s Object Space and selects a slot that
contains either a Null Capability (CAP 0) or an Object Capability (CAP OBJ) that refers to a kernel object with
associated access permissions.
2.1.2 Host Space
1. Only one Host Space (SPC HST) can be created per Protection Domain.
2. Access to a Host Space is controlled by a Host Space Capability (CAP OBJ HST).
3. Each memory operation issued by a Host Execution Context implicitly uses the page number of the accessed
Host-Virtual Address (HV A) as Host Capability Selector: SEL HST=HV A >>12.
4. The Host Capability Selector (SEL HST) serves as index into the EC’s Host Space and selects a slot that
contains either a Null Capability (CAP 0) or a Memory Capability (CAP MEM) that refers to a 4 KiB page
frame in physical memory with associated access permissions.
2.1.3 Guest Space
1. Multiple Guest Spaces (SPC GST) can be created per Protection Domain.
2. Access to a Guest Space is controlled by a Guest Space Capability (CAP OBJ GST).
3. Each memory operation issued by a Guest Execution Context implicitly uses the page number of the accessed
Guest-Physical Address (GPA) as Guest Capability Selector: SEL GST=GPA >>12.
4. The Guest Capability Selector (SEL GST) serves as index into the EC’s Guest Space and selects a slot that
contains either a Null Capability (CAP 0) or a Memory Capability (CAP MEM) that refers to a 4 KiB page
frame in physical memory with associated access permissions.
4

2.1.4 DMA Space
1. Multiple DMA Spaces (SPC DMA) can be created per Protection Domain.
2. Access to a DMA Space is controlled by a DMA Space Capability (CAP OBJ DMA).
3. Each DMA operation issued by a device implicitly uses the page number of the accessed DMA-Virtual
Address (DV A) as DMA Capability Selector: SEL DMA=DV A >>12.
4. The DMA Capability Selector (SEL DMA) serves as index into the device’s DMA Space and selects a slot
that contains either a Null Capability (CAP 0) or a Memory Capability (CAP MEM) that refers to a 4 KiB page
frame in physical memory with associated access permissions.
2.1.5 PIO Space
1. Multiple PIO Spaces (SPC PIO) can be created per Protection Domain.
2. Access to a PIO Space is controlled by a PIO Space Capability (CAP OBJ PIO).
3. Each PIO access ( IN/OUT instruction) issued by an Execution Context (EC) implicitly uses the number of
the accessed I /O port as PIO Capability Selector.
4. The PIO Capability Selector (SEL PIO) serves as index into the EC’s PIO Space and selects a slot that
contains either a Null Capability (CAP 0) or a PIO Capability (CAP PIO) that refers to the I /O port SEL PIO
with associated access permissions.
2.1.6 MSR Space
1. Multiple MSR Spaces (SPC MSR) can be created per Protection Domain.
2. Access to an MSR Space is controlled by an MSR Space Capability (CAP OBJ MSR).
3. Each MSR access ( RDMSR/WRMSR instruction) issued by an Execution Context (EC) implicitly uses the
number of the accessed MSR as MSR Capability Selector.
4. The MSR Capability Selector (SEL MSR) serves as index into the EC’s MSR Space and selects a slot that
contains either a Null Capability (CAP 0) or an MSR Capability (CAP MSR) that refers to the MSR SEL MSR
with associated access permissions.
5

2.2 Execution Context
1. The Execution Context (EC) is an abstraction for an activity within a PD.
2. Access to an Execution Context is controlled by an EC Capability (CAP OBJ EC).
3. An EC is permanently bound to one CPU.
4. An EC contains architecture-dependent state, such as
•Central Processing Unit (CPU) registers
•Floating Point Unit (FPU) registers (optionally)
The following subsections provide more details for each type of Execution Context.
2.2.1 Host Execution Context
1. There exist two types of Host Execution Context (EC HST):
•Local Threads – these may have PTs (but no SCs) bound to it.
•Global Threads – these may have an SC (but no PTs) bound to it.
2. A Host Execution Context has a UTCB that enables it to perform regular IPC.
3. Upon creation, a Host Execution Context is permanently bound to the following required spaces of its PD:
•The first Object Space *
•The first Host Space *
•The first PIO Space†
4. A Host Execution Context cannot be reassigned to any spaces.
2.2.2 Guest Execution Context
1. There exists one type of Guest Execution Context (EC GST):
•Virtual CPUs – these may have an SC (but no PTs) bound to it.
2. A Guest Execution Context does not have a UTCB.
3. Upon creation, a Guest Execution Context is permanently bound to the following required spaces of its PD:
•The first Object Space *
•The first Host Space *
4. Upon any architectural /microhypervisor event, a Guest Execution Context may be (re)assigned to the
following spaces of its PD, by setting the SPACES bit in the MTD and providing selectors that refer to
capabilities with ASSIGN permission for these spaces in the respective UTCB fields during ipc_reply :
•Any Guest Space†
•Any PIO Space†
•Any MSR Space†
The handler of the STARTUP event must perform the initial assignment of these spaces.
*Because only one such space can be created for a PD, it is also the only such space.
†Only on architectures, where this type of space is available.
6

2.3 Scheduling Context
1. The Scheduling Context (SC) is a unit of prioritization and temporal isolation.
2. Access to a Scheduling Context is controlled by an SC Capability (CAP OBJ SC).
3. An SC is permanently bound to one CPU.
4. An SC is permanently bound to the EC for which it was created.
5. Donation allows another EC to consume the budget of the SC for the duration of the donation.
6. A scheduling context comprises the following state:
•Reference to bound EC (2.2)
•Class Of Service (COS)
•Priority – numerically higher priorities always preempt numerically lower priorities
•Budget – time after which the SC can be preempted by an SC with the same priority
2.4 Portal
1. A Portal (PT) represents a dedicated entry point into the PD for which the portal was created.
2. Access to a Portal is controlled by a PT Capability (CAP OBJ PT).
3. A PT is permanently bound to the EC for which it was created.
4. A portal comprises the following state:
•Reference to bound EC (2.2)
•Message Transfer Descriptor (MTD) (4.4)
•Entry Instruction Pointer (IP)
•Portal Identifier (PID)
2.5 Semaphore
1. A Semaphore (SM) provides a means to synchronize execution and interrupt delivery by selectively blocking
and unblocking Execution Contexts (ECs).
2. Access to a Semaphore is controlled by a SM Capability (CAP OBJ SM).
2.6 Device Context
1. A Device Context (DC) provides management information for a hardware device.
2. Access to a Device Context is controlled by a DC Capability (CAP OBJ DC).
3. A Device Context comprises the following state:
•Topology of the device, such as PCI Segment /Bus/Device /Function (SBDF)
•Reference to the component that performs DMA translation for the device, such as IOMMU or SMMU
•Reference to the component that performs MSI translation for the device, such as IOMMU or GITS
•Auxiliary information
7

3 Hardware Resources
3.1 System Time Counter
The system time is represented by an unsigned 64-bit System Time Counter (STC) with the following properties:
1. The STC starts with a power-on value of 0.
2. Subsequent reads of the STC return a higher value that reflects the platform uptime.
3. While the platform is in a shallow sleep state, the STC retains its current value.
4. While the platform is running, the STC monotonically increments at a fixed frequency, which is conveyed in
the Hypervisor Information Page (HIP).
5. The STC and its frequency are synchronized across all CPUs. Applications can use both values to convert
between system time and wall clock time.
6. Applications can obtain the current STC value as follows:
Arm: By reading CNTVCT_EL0 via theMRSinstruction [3].
x86: By reading IA32_TSC via theRDTSC instruction [4, 5].
8

Part III
Application Programming Interface
9

4 Data Types
4.1 Capability
Capabilities are communicable, unforgeable tokens of authority. They consist of a reference to a resource coupled
with access permissions. Capabilities are opaque and immutable for applications – they cannot be inspected or
modified directly; instead applications refer to a Capability (CAP) via a Capability Selector (SEL).
4.1.1 Null Capability
A Null Capability (CAP 0) does not refer to anything and carries no permissions.
4.1.2 Object Capability
An Object Capability (CAP OBJ) is stored in the Object Space of a PD and refers to a kernel object.
4.1.2.1 Object Space Capability
An Object Space Capability (CAP OBJ OBJ) refers to an Object Space and carries the following permissions:–
–
–
–
TAKE
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that Object Space as destination.
TAKE If set,ctrl_pd can use that Object Space as source.
4.1.2.2 Host Space Capability
A Host Space Capability (CAP OBJ HST) refers to a Host Space and carries the following permissions:–
–
–
–
TAKE
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that Host Space as destination.
TAKE If set,ctrl_pd can use that Host Space as source.
4.1.2.3 Guest Space Capability
A Guest Space Capability (CAP OBJ GST) refers to a Guest Space and carries the following permissions:–
–
–
ASSIGN
–
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that Guest Space as destination.
ASSIGN If set,ipc_reply can assign an EC GSTto that Guest Space.
10

4.1.2.4 DMA Space Capability
A DMA Space Capability (CAP OBJ DMA) refers to a DMA Space and carries the following permissions:–
–
–
ASSIGN
–
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that DMA Space as destination.
ASSIGN If set,assign_dev can assign a device to that DMA Space.
4.1.2.5 PIO Space Capability
A PIO Space Capability (CAP OBJ PIO) refers to a PIO Space and carries the following permissions:–
–
–
ASSIGN
TAKE
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that PIO Space as destination.
TAKE If set,ctrl_pd can use that PIO Space as source.
ASSIGN If set,ipc_reply can assign an EC GSTto that PIO Space.
4.1.2.6 MSR Space Capability
An MSR Space Capability (CAP OBJ MSR) refers to an MSR Space and carries the following permissions:–
–
–
ASSIGN
TAKE
GRANT
0 1 2 3 4 5GRANT If set,ctrl_pd can use that MSR Space as destination.
TAKE If set,ctrl_pd can use that MSR Space as source.
ASSIGN If set,ipc_reply can assign an EC GSTto that MSR Space.
4.1.2.7 Protection Domain Capability
A PD Capability (CAP OBJ PD) refers to a Protection Domain (PD) and carries the following permissions:DC
SM
PT
SC
EC
PD
0 1 2 3 4 5PD If set,create_pd permitted.
EC If set,create_ec permitted.
SC If set,create_sc permitted.
PT If set,create_pt permitted.
SM If set,create_sm permitted.
DC If set,create_dc permitted.
4.1.2.8 Execution Context Capability
An EC Capability (CAP OBJ EC) refers to an Execution Context (EC) and carries the following permissions:–
–
BINDSC
BINDPT
–
CTRL
0 1 2 3 4 5CTRL If set,ctrl_ec permitted.
BINDPT If set,create_pt can bind a Portal (PT) to the EC.
BINDSC If set,create_sc can bind a Scheduling Context (SC) to the EC.
11

4.1.2.9 Scheduling Context Capability
An SC Capability (CAP OBJ SC) refers to a Scheduling Context (SC) and carries the following permissions:–
–
–
–
–
CTRL
0 1 2 3 4 5CTRL If set,ctrl_sc permitted.
4.1.2.10 Portal Capability
A PT Capability (CAP OBJ PT) refers to a Portal (PT) and carries the following permissions:–
–
–
EVENT
CALL
CTRL
0 1 2 3 4 5CTRL If set,ctrl_pt permitted.
CALL If set,ipc_call permitted.
EVENT If set, delivery of events permitted.
4.1.2.11 Semaphore Capability
An SM Capability (CAP OBJ SM) refers to a Semaphore (SM) and carries the following permissions:–
–
–
ASSIGN
CTRLDN
CTRLUP
0 1 2 3 4 5CTRLUP If set,ctrl_sm (Up) permitted.
CTRLDN If set,ctrl_sm (Down ) permitted.
ASSIGN†If set,assign_int permitted.
4.1.2.12 Device Context Capability
A DC Capability (CAP OBJ DC) refers to a Device Context (DC) and carries the following permissions:–
–
–
–
ASSIGN INT
ASSIGN DEV
0 1 2 3 4 5ASSIGN DEV If set,assign_dev to a DMA Space permitted.
ASSIGN INT If set,assign_int to an Interrupt Semaphore permitted.
4.1.3 Memory Capability
A Memory Capability (CAP MEM) is stored in a Host Space, Guest Space or DMA Space of a PD, refers to a 4 KiB
page frame, and carries the following permissions:
–
–
XS
XU
W
R
0 1 2 3 4 5R If set, the page frame is readable.
W If set, the page frame is writable.
XU‡If set, the page frame is executable (in user mode).
XS‡If set, the page frame is executable (in supervisor mode).
†This permission bit is only defined for interrupt semaphores.
‡If the hardware supports only combined execute permissions ( X) for both modes, then X=XU∨XS.
12

4.1.4 PIO Capability
A PIO Capability (CAP PIO) is stored in a PIO Space of a PD, refers to an I /O port, and carries the following
permissions:
–
–
–
–
–
A
0 1 2 3 4 5A If set, the I /O port is accessible (via IN/OUT).
4.1.5 MSR Capability
An MSR Capability (CAP MSR) is stored in an MSR Space of a PD, refers to a Model-Specific Register (MSR),
and carries the following permissions:
–
–
–
–
W
R
0 1 2 3 4 5R If set, the MSR is readable (via RDMSR ).
W If set, the MSR is writable (via WRMSR ).
13

4.2 Capability Selector
A Capability Selector (SEL) is an application-visible unsigned number that serves as an index into a space of an
Execution Context to select a Capability (CAP). The following subsections provide more details.
4.2.1 Object Capability Selector
1. Object Spaces (SPC OBJ) are unavailable if SBW OBJ=0. No systems currently report this.
2. Otherwise, a valid Object Capability Selector (SEL OBJ) is in range0,2SBW OBJ.
4.2.2 Host Capability Selector
1. Host Spaces (SPC HST) are unavailable if SBW HST=0. No systems currently report this.
2. Otherwise, a valid Host Capability Selector (SEL HST) is in range0,2SBW HST. =⇒valid HV A range
4.2.3 Guest Capability Selector
1. Guest Spaces (SPC GST) are unavailable if SBW GST=0. Systems without CPU virtualization may report this.
2. Otherwise, a valid Guest Capability Selector (SEL GST) is in range0,2SBW GST. =⇒valid GPA range
4.2.4 DMA Capability Selector
1. DMA Spaces (SPC DMA) are unavailable if SBW DMA=0. Systems without I /O virtualization may report this.
2. Otherwise, a valid DMA Capability Selector (SEL DMA) is in range0,2SBW DMA. =⇒valid DV A range
4.2.5 PIO Capability Selector
1. PIO Spaces (SPC PIO) are unavailable if SBW PIO=0. Non-x86 systems may report this.
2. Otherwise, a valid PIO Capability Selector (SEL PIO) is in range0,2SBW PIO. =⇒valid PIO range
4.2.6 MSR Capability Selector
1. MSR Spaces (SPC MSR) are unavailable if SBW MSR=0. Non-x86 systems may report this.
2. Otherwise, a valid MSR Capability Selector (SEL MSR) is in range0,2SBW MSR. =⇒valid MSR range
14

4.3 User Thread Control Block
A User Thread Control Block (UTCB) has a size of one memory page (4 KiB) and is mapped in the Host Space of
its associated Host Execution Context. Because a UTCB is allocated and owned by the microhypervisor, it cannot
be delegated using ctrl_pd .
To ensure proper visibility of loads and stores with relaxed memory ordering, application programs are expected
to access a UTCB only from the Host Execution Context to which that UTCB is bound.
4.3.1 Regular Layout
During regular IPC (see 4.4.1), the UTCB is used for data transfer.
The data transfer from one UTCB to another UTCB is defined as follows:
•The data transfer is performed by the CPU on which the caller EC and callee EC execute.
•The data transfer uses the regular layout with 512 message words (see below).
•The data is copied from low words to high words, beginning with word0.
•The granularity of the loads and stores used for copying is undefined .
•Loads from and stores to the UTCB are non-atomic and use relaxed memory ordering.
0 63+0x1000
word511 +0x0ff8
word510 +0x0ff0 hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
+0x0010
word1 +0x0008
word0 +0x0000Message
Words
0 63
4.3.2 Architectural Layout
During architectural IPC (see 4.4.2), the UTCB is used for state transfer.
The state transfer between the architectural registers and a UTCB is defined as follows:
•The state transfer is performed by the CPU on which the a ffected EC and callee EC execute.
•The state transfer uses the architectural layout (Arm, x86).
•The state is copied between architectural registers and the UTCB in an undefined order.
•The granularity of the loads and stores used for copying is undefined .
•Loads from and stores to the UTCB are non-atomic and use relaxed memory ordering.
15

4.4 Message Transfer Descriptor
4.4.1 Regular IPC
For regular Inter-Process Communication (IPC), the Message Transfer Descriptor (MTD) is provided by the sender,
passed to the receiver, and uses the following layout:
– UTCB Message Words - 1
0 8 9 31
The MTD controls the data transfer (see 4.3.1) as shown in Figure 4.1:
•Duringipc_call , it specifies the number of message words to transfer from the UTCB of the
caller EC (sender) to the UTCB of the callee EC (receiver).
•Duringipc_reply , it specifies the number of message words to transfer from the UTCB of the
callee EC (sender) to the UTCB of the caller EC (receiver).
SC SC
PTECcallee ECcaller
NOV A Microhypervisor NOV A MicrohypervisorUTCB UTCB UTCB UTCBECcallee ECcallerPDA PDB PDA PDB
ipc_reply (MTD) ipc_call (PT,MTD)
Figure 4.1: Regular IPC
4.4.2 Architectural IPC
For exceptions and intercepts, the Message Transfer Descriptor (MTD) is provided by the architectural event-
specific portal (Arm, x86) or sender, passed to the receiver, and uses an architectural bitfield layout (Arm, x86):
•If a bit is0, then the microhypervisor does nottransmit the architectural state associated with that bit.
•If a bit is1, then the microhypervisor transmits the architectural state associated with that bit.
The MTD controls the state transfer (see 4.3.2) as shown in Figure 4.2:
•During an exception /intercept, it specifies the subset of registers to transfer from the architectural state of
the affected EC (sender) to the UTCB of the callee EC (receiver).
•Duringipc_reply , it specifies the subset of registers to transfer from the UTCB of the callee EC (sender)
to the architectural state of the a ffected EC (receiver).
SC SC
PT
NOV A Microhypervisor NOV A MicrohypervisorEXCEPTION/INTERCEPT RETURNPDA PDB
ECcalleePDA
ECaffectedPDB
ECcallee ECaffected
StateUTCB UTCB
Stateimplicit_call (PT,MTD PT)ipc_reply (MTD)
Figure 4.2: Architectural IPC
16

4.5 Scheduling Context Descriptor
The Scheduling Context Descriptor (SCD) describes the configuration of a Scheduling Context (SC).
– COS Prio Budget
0 15 16 2223 38 39 63
The fields are defined as follows:
Budget
Specifies the scheduling budget in milliseconds – must be >0.
Prio
Specifies the scheduling priority – must be >0.
COS
Specifies the Class Of Service – valid values depend on architectural COS support (Arm, x86):
•If COS is not supported, then this field must be 0.
•If COS is supported, then this field must be <COS NUM.
17

5 Hypercalls
5.1 Definitions
5.1.1 Hypercall Numbers
Each hypercall is identified by a unique number. The following hypercalls are currently defined:
Number Hypercall Section
0x0 ipc_call 5.2.1
0x1 ipc_reply 5.2.2
0x2 create_pd 5.3.1
0x3 create_ec 5.3.2
0x4 create_sc 5.3.3
0x5 create_pt 5.3.4
0x6 create_sm 5.3.5
0x7 create_dc 5.3.6
0x8 ctrl_pd 5.4.1
0x9 ctrl_ec 5.4.2
0xa ctrl_sc 5.4.3
0xb ctrl_pt 5.4.4
0xc ctrl_sm 5.4.5
0xd ctrl_hw 5.5.1
0xe assign_dev 5.5.2
0xf assign_int 5.5.3
5.1.2 Status Codes
Hypercalls return a status code to indicate success or failure. The following status codes are currently defined:
Number Status Code Description
0x0 SUCCESS Operation Successful
0x1 TIMEOUT Operation Timeout
0x2 ABORTED Operation Abort
0x3 OVRFLOW Operation Overflow
0x4 BAD_HYP Invalid Hypercall
0x5 BAD_CAP Invalid Capability
0x6 BAD_PAR Invalid Parameter
0x7 BAD_FTR Invalid Feature
0x8 BAD_CPU Invalid CPU Number
0x9 BAD_DEV Invalid Device ID
0xa MEM_OBJ Insufficient Memory (Object Creation)
0xb MEM_CAP Insufficient Memory (Capability Creation)
≥0xc reserved for future use
18

5.2 Communication
5.2.1 IPC Call
Parameters:
status = ipc_call (SEL OBJpt, // Portal
MTD& mtd) // Message Transfer Descriptor
Flags:
000T
0 1 2 3
Description:
Sends a message from EC CURRENT (caller) to the EC (callee) to which the specified Portal (PT) is bound.
Prior to the hypercall:
•SPC OBJ CURRENTptmust refer to a PT Capability (CAP OBJ PT) with permission CALL .
If the hypercall completed successfully:
•IfT=0 (No Timeout) : If the callee EC was still busy handling a prior ipc_call , then the caller EC
has helped run that prior ipc_call to completion, i.e. until the callee EC became available again.
•The microhypervisor has transferred a message from the UTCB of the caller EC to the UTCB of the
callee EC. The content of that message is defined by the MTD mtd, which has been passed from the
caller EC to the callee EC.
•The hypercall returns once the callee EC has invoked an ipc_reply . Upon return, the UTCB of the
caller EC and the mtdparameter have been updated by the reply message.
•The Current Scheduling Context (SC CURRENT ) has been donated to the callee EC upon ipc_call and
returned back upon ipc_reply , thereby accounting the entire handling of the request to SC CURRENT .
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CAP
•SPC OBJ CURRENTptdid not refer to a PT Capability (CAP OBJ PT) or that capability had insu fficient
permissions.
BAD_CPU
•Caller EC and callee EC are on di fferent CPUs.
TIMEOUT
•IfT=1 (Timeout) : The callee EC is still busy handling a prior ipc_call .
ABORTED
•The callee EC is dead and the operation aborted.
19

5.2.2 IPC Reply
Parameters:
pid = ipc_reply (MTD& mtd) // Message Transfer Descriptor
Flags:
0000
0 1 2 3
Description:
Sends a reply message from EC CURRENT (callee) back to the caller EC (if one exists) and subsequently waits
for the next incoming message.
If the hypercall completed successfully:
•If a caller EC exists:
–The microhypervisor has transferred a reply message from the UTCB of the callee EC back to the
UTCB of the caller EC.
–The content of that reply message is defined by the MTD mtd, which has been passed from the
callee EC back to the caller EC.
–The Current Scheduling Context (SC CURRENT ) that had been donated to the callee EC upon
ipc_call has been returned back to the caller EC.
•ECCURRENT blocks until the next incoming message arrives on any Portal (PT) bound to it.
Status:
This hypercall does not return directly.
Instead, when the next message arrives via a subsequent ipc_call to any Portal (PT) bound to the callee EC:
•The microhypervisor passes the Portal Identifier (PID) of the called PT to the callee EC.
•The UTCB of the callee EC and the mtdparameter have been updated by the incoming message.
•Execution of the callee EC continues at the Instruction Pointer (IP) configured in the called PT.
20

5.3 Object Creation
5.3.1 Create Protection Domain
Parameters:
status = create_pd (SEL OBJsel, // Created PD or Space
SEL OBJpd) // Owner PD
Flags:
OP
0 3
Description:
Creates a new Protection Domain (PD) or an empty new space for a Protection Domain.
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission PD.
If the hypercall completed successfully:
•IfOP=0 (Protection Domain) :
–A new Protection Domain (PD) has been created.
–SPC OBJ CURRENTselrefers to a CAP OBJ PDfor the created Protection Domain with the permissions
inherited from SPC OBJ CURRENTpd.
•IfOP=1 (Object Space) :
–A new Object Space (SPC OBJ) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ OBJfor the created Object Space with all defined permissions.
•IfOP=2 (Host Space) :
–A new Host Space (SPC HST) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ HSTfor the created Host Space with all defined permissions.
•IfOP=3 (Guest Space) :
–A new Guest Space (SPC GST) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ GSTfor the created Guest Space with all defined permissions.
•IfOP=4 (DMA Space) :
–A new DMA Space (SPC DMA) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ DMAfor the created DMA Space with all defined permissions.
•IfOP=5 (PIO Space) :
–A new PIO Space (SPC PIO) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ PIOfor the created PIO Space with all defined permissions.
•IfOP=6 (MSR Space) :
–A new MSR Space (SPC MSR) has been created for the PD referred to by SPC OBJ CURRENTpd.
–SPC OBJ CURRENTselrefers to a CAP OBJ MSRfor the created MSR Space with all defined permissions.
•The resources for the created object were accounted to the PD referred to by SPC OBJ CURRENTpd.
Status:
SUCCESS
•The hypercall completed successfully.
21

ABORTED
•The PD designated by SPC OBJ CURRENTpdis being destructed concurrently.
•IfOP>0: The space could not bind to the PD designated by SPC OBJ CURRENTpd.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
BAD_PAR
•The requested operation (OP) is invalid.
BAD_FTR
•The requested space type is not supported by the hardware architecture.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
22

5.3.2 Create Execution Context
Parameters:
status = create_ec (SEL OBJsel, // Created EC
SEL OBJpd, // Owner PD
SEL HSThvp, // Host-Virtual Page Number
SEL EVTevt, // Event Selector Base
UINT16cpu, // CPU Number
UINTPTRsp) // Initial Stack Pointer
Flags:
0FTG
0 1 2 3
Description:
Creates a new Execution Context (EC).
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission EC.
•IfG=0: All spaces required for a Host Execution Context must have been created for the owner PD.
•IfG=1: All spaces required for a Guest Execution Context must have been created for the owner PD.
If the hypercall completed successfully:
•IfG=0 (Host Execution Context) :
–A new Host Execution Context (EC HST) has been created.
–The microhypervisor has allocated a UTCB for EC HSTand mapped it at HV A hvp <<12.
–IfT=0 (Local Thread) : Portals (PTs) may subsequently be bound to EC HST, which will run
whenever any of those bound portals is called.
–IfT=1 (Global Thread) : EC HSTwill generate a STARTUP event the first time a Scheduling
Context (SC) is bound to it.
•IfG=1 (Guest Execution Context) :
–A new Guest Execution Context (EC GST) has been created.
–The microhypervisor has allocated a vAPIC page for EC GSTand mapped it at HV A hvp <<12.
On non-Intel architectures, the parameter hvpwas ignored.
–ECGSTwill generate a STARTUP event the first time a Scheduling Context (SC) is bound to it.
–IfT=0 (Virtual CPU) : The virtual CPU uses no time adjustment.
–IfT=1 (Virtual CPU) : The virtual CPU uses time o ffsetting.
•The created EC will be able to use FPU instructions only if F=1 (FPU) . Otherwise any FPU access by
that EC will generate an architecture-specific exception.
•The created EC has its Stack Pointer (SP) set to spand its Event Selector Base (SEL EVT) set toevt.†
•The created EC is permanently bound to the CPU cpuand to the required spaces of the PD referred to
by SPC OBJ CURRENTpd.
•The resources for the created EC were accounted to the PD referred to by SPC OBJ CURRENTpd.
•SPC OBJ CURRENTselrefers to a CAP OBJ ECfor the created EC with all defined permissions.
†The microhypervisor sets these values only once during EC creation. Subsequently, each Host Execution Context is responsible for
maintaining its Stack Pointer (SP) across hypercalls. Applications can use di fferent initial SP or SEL EVTvalues as a means to identify
each Host Execution Context or Guest Execution Context during concurrent STARTUP events.
23

Status:
SUCCESS
•The hypercall completed successfully.
ABORTED
•A required space of the PD designated by SPC OBJ CURRENTpdis being destructed concurrently.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
BAD_CPU
•The CPU number is invalid.
BAD_FTR
•Virtual CPUs are not supported by the hardware architecture.
BAD_PAR
•The HV A corresponding to hvpis outside the user-accessible memory range.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
24

5.3.3 Create Scheduling Context
Parameters:
status = create_sc (SEL OBJsel, // Created SC
SEL OBJpd, // Owner PD
SEL OBJec, // Bound EC
SCD scd) // Scheduling Context Descriptor
Flags:
0000
0 1 2 3
Description:
Creates a new Scheduling Context (SC).
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission SC.
•SPC OBJ CURRENTecmust refer to an EC Capability (CAP OBJ EC) with permission BINDSC.
•The owner PD designated by CAP OBJ PDand the owner PD of the bound EC must be the same.
If the hypercall completed successfully:
•A new Scheduling Context (SC) has been created.
•The created SC is bound to the EC referred to by SPC OBJ CURRENTecon the CPU of that EC, with its
scheduling parameters set according to scd.
•The resources for the created SC were accounted to the PD referred to by SPC OBJ CURRENTpd.
•SPC OBJ CURRENTselrefers to a CAP OBJ SCfor the created SC with all defined permissions.
Status:
SUCCESS
•The hypercall completed successfully.
ABORTED
•The EC designated by SPC OBJ CURRENTecis being destructed concurrently.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
•SPC OBJ CURRENTecdid not refer to an EC Capability (CAP OBJ EC) or that capability had insu fficient
permissions.
•Binding the SC to the EC failed, e.g. because the EC is a local thread.
BAD_PAR
•At least one SCD field in scdwas invalid.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
25

5.3.4 Create Portal
Parameters:
status = create_pt (SEL OBJsel, // Created PT
SEL OBJpd, // Owner PD
SEL OBJec, // Bound EC
UINTPTRip) // Instruction Pointer
Flags:
0000
0 1 2 3
Description:
Creates a new Portal (PT).
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission PT.
•SPC OBJ CURRENTecmust refer to an EC Capability (CAP OBJ EC) with permission BINDPT.
•The owner PD designated by CAP OBJ PDand the owner PD of the bound EC must be the same.
If the hypercall completed successfully:
•A new Portal (PT) has been created.
•The created PT is bound to the EC referred to by SPC OBJ CURRENTecon the CPU of that EC, with its
Portal Instruction Pointer (IP) set to ip, its initial MTD set to 0 and its initial PID set to 0.
•The resources for the created PT were accounted to the PD referred to by SPC OBJ CURRENTpd.
•SPC OBJ CURRENTselrefers to a CAP OBJ PTfor the created PT with all defined permissions.
Status:
SUCCESS
•The hypercall completed successfully.
ABORTED
•The EC designated by SPC OBJ CURRENTecis being destructed concurrently.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
•SPC OBJ CURRENTecdid not refer to an EC Capability (CAP OBJ EC) or that capability had insu fficient
permissions.
•Binding the PT to the EC failed, e.g. because the EC is not a local thread.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
26

5.3.5 Create Semaphore
Parameters:
status = create_sm (SEL OBJsel, // Created SM
SEL OBJpd, // Owner PD
UINT64val) // Initial Counter Value / ISD
Flags:
000I
0 1 2 3
Description:
Creates a new Semaphore (SM).
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•IfI=0 (Regular Semaphore)
–SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission SM.
•IfI=1 (Interrupt Semaphore)
–SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission DC.
–The Interrupt Semaphore Descriptor (ISD) must be valid.
If the hypercall completed successfully:
•IfI=0 (Regular Semaphore)
–A new regular Semaphore (SM) has been created.
–The regular Semaphore has an initial counter value of val.
–The regular Semaphore is not associated with any interrupt.
•IfI=1 (Interrupt Semaphore)
–A new interrupt Semaphore (SM) has been created.
–The interrupt Semaphore has an initial counter value of 0.
–The interrupt Semaphore is permanently associated with the interrupt designated by ISD.
•The resources for the created SM were accounted to the PD referred to by SPC OBJ CURRENTpd.
•SPC OBJ CURRENTselrefers to a CAP OBJ SMfor the created SM with all defined permissions.
Status:
SUCCESS
•The hypercall completed successfully.
ABORTED
•The PD designated by SPC OBJ CURRENTpdis being destructed concurrently.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
BAD_PAR
•IfI=1: The Interrupt Semaphore Descriptor (ISD) was invalid.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
27

•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
28

5.3.6 Create Device Context
Parameters:
status = create_dc (SEL OBJsel, // Created DC
SEL OBJpd, // Owner PD
DTD dtd) // Device Topology Descriptor
Flags:
0000
0 1 2 3
Description:
Creates a new Device Context (DC).
Prior to the hypercall:
•SPC OBJ CURRENTselmust refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpdmust refer to a PD Capability (CAP OBJ PD) with permission DC.
If the hypercall completed successfully:
•A new Device Context (DC) has been created.
•The created DC has its topology information configured according to DTD.
•The resources for the created DC were accounted to the PD referred to by SPC OBJ CURRENTpd.
•SPC OBJ CURRENTselrefers to a CAP OBJ DCfor the created DC with all defined permissions.
Status:
SUCCESS
•The hypercall completed successfully.
ABORTED
•The PD designated by SPC OBJ CURRENTpdis being destructed concurrently.
BAD_CAP
•SPC OBJ CURRENTseldid not refer to a Null Capability (CAP 0).
•SPC OBJ CURRENTpddid not refer to a PD Capability (CAP OBJ PD) or that capability had insu fficient
permissions.
MEM_OBJ
•The Protection Domain referred to by SPC OBJ CURRENTpdhad insu fficient memory resources for
object creation.
MEM_CAP
•The Protection Domain to which SPC OBJ CURRENT belongs had insu fficient memory resources for
capability creation.
29

5.4 Object Control
5.4.1 Control Protection Domain
Parameters:
status = ctrl_pd (SEL OBJsrc, // SRC Space
SEL OBJdst, // DST Space
SEL ssb, // SRC Selector Base
SEL dsb, // DST Selector Base
UINT5ord, // Order
UINT6pmm, // Permission Mask
MAD mad) // Memory Attribute Descriptor
Flags:
0000
0 1 2 3
Description:
Delegates capabilities from the specified selector range in the source space to the specified selector range in
the destination space and thereby optionally reduces the permissions for the destination capabilities.
Capability delegation is defined as follows:
1. Capabilities are delegated from source Capability Selector range srcssb,ssb+2ord.
2. Capabilities are delegated to destination Capability Selector range dstdsb,dsb+2ord.
3. The permissions of each destination capability are the logical AND of the permissions of the respective
source capability and the permission mask pmm, i.e.
a) for bits set (1) in pmm, the respective permissions are inherited from the source capability.
b) for bits clear (0) in pmm, the respective permissions are removed for the destination capability.
4. The destination capability is always a Null Capability (CAP 0) in the following cases:
a) if the source capability is a Null Capability (CAP 0).
b) if the destination capability obtains zero permissions from the source capability.
c) if the object that the source capability refers to is being destructed concurrently.
Prior to the hypercall:
•SPC OBJ CURRENTsrcmust refer to a CAP OBJfor the source space with permission TAKE .
•SPC OBJ CURRENTdstmust refer to a CAP OBJfor the destination space with permission GRANT .
•Capability Selectors ssbanddsbmust be order-aligned: ssb≡0 (mod 2ord)anddsb≡0 (mod 2ord).
•Capability Selectors ssbanddsbmust be equal if srcrefers to a PIO Space or an MSR Space.
If the hypercall completed successfully:
•If bothsrcanddstrefer to Object Spaces:
–All CAP OBJ(or CAP 0) from the source selector range were delegated to the destination selector
range. Any pre-existing CAP OBJin the destination selector range were revoked.
–The parameter madwas ignored.
•Ifsrcrefers to a Host Space and dstrefers to a Host Space or Guest Space or DMA Space:
–All CAP MEM (or CAP 0) from the source selector range were delegated to the destination selector
range. Any pre-existing CAP MEM in the destination selector range were revoked.
–Ifsrc refers to the NOV A Host Space, then source SEL HSTarephysical page numbers and the
memory attributes of each destination CAP MEM were setaccording to mad.
30

Otherwise, source SEL HSTarevirtual page numbers and the memory attributes of each destina-
tion CAP MEM were inherited from the respective source CAP MEM, i.e. the parameter mad was
ignored.
•If bothsrcanddstrefer to PIO Spaces:
–All CAP PIO(or CAP 0) from the source selector range were delegated to the destination selector
range. Any pre-existing CAP PIOin the destination selector range were revoked.
–The parameter madwas ignored.
•If bothsrcanddstrefer to MSR Spaces:
–All CAP MSR(or CAP 0) from the source selector range were delegated to the destination selector
range. Any pre-existing CAP MSRin the destination selector range were revoked.
–The parameter madwas ignored.
•The resources for storing the granted capabilities were accounted to the PD to which the space referred
to by SPC OBJ CURRENTdstbelongs.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CAP
•SPC OBJ CURRENTsrcor SPC OBJ CURRENTdstdid not refer to compatible space capabilities or had
insufficient permissions.
BAD_PAR
•Capability Selector ssbordsbwas not order-aligned.
•Capability Selector ssb+2ordordsb+2ordwas larger than the number of selectors.
•Ifsrcrefers to a PIO Space or an MSR Space: Capability Selectors ssbanddsbwere not equal.
•Ifsrcrefers to the NOV A Host Space: At least one MAD field in madwas invalid.
MEM_CAP
•The PD to which the space referred to by SPC OBJ CURRENTdstbelongs had insu fficient memory
resources for allocating the storage required for granting all destination capabilities. This
constitutes a partial failure of the operation, because all destination capabilities up to the first
allocation failure have been granted.
31

5.4.2 Control Execution Context
Parameters:
status = ctrl_ec (SEL OBJec) // Execution Context
Flags:
000S
0 1 2 3
Description:
Prior to the hypercall:
•SPC OBJ CURRENTecmust refer to an EC Capability (CAP OBJ EC) with permission CTRL .
If the hypercall completed successfully:
•The EC referred to by SPC OBJ CURRENTechas been forced to enter the microhypervisor. It will generate a
recall exception prior to its next exit from the microhypervisor and will traverse through the respective
Event Portal (Arm, x86).
•IfS=0 (Weak Recall) :
–The hypercall returns as soon as the recall exception has been pended , i.e. the EC may not have
entered the microhypervisor yet.
•IfS=1 (Strong Recall) :
–The hypercall returns as soon as the recall exception has been observed , i.e the EC will have
entered the microhypervisor.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CAP
•SPC OBJ CURRENTecdid not refer to an EC Capability (CAP OBJ EC) or that capability had insu fficient
permissions.
32

5.4.3 Control Scheduling Context
Parameters:
status = ctrl_sc (SEL OBJsc, // Scheduling Context
UINT64&stc) // Total Consumed Execution Time
Flags:
0000
0 1 2 3
Description:
Prior to the hypercall:
•SPC OBJ CURRENTscmust refer to an SC Capability (CAP OBJ SC) with permission CTRL .
If the hypercall completed successfully:
•The microhypervisor has returned the total consumed execution time as System Time Counter (STC)
value for the SC referred to by SPC OBJ CURRENTsc.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CAP
•SPC OBJ CURRENTscdid not refer to an SC Capability (CAP OBJ SC) or that capability had insu fficient
permissions.
33

5.4.4 Control Portal
Parameters:
status = ctrl_pt (SEL OBJpt, // Portal
UINTPTRpid, // Portal Identifier
MTD mtd) // Message Transfer Descriptor
Flags:
0000
0 1 2 3
Description:
Prior to the hypercall:
•SPC OBJ CURRENTptmust refer to a PT Capability (CAP OBJ PT) with permission CTRL .
If the hypercall completed successfully:
•The microhypervisor has set the Portal Identifier (PID) to pid and the Message Transfer Descriptor
(MTD) to mtdfor the Portal referred to by SPC OBJ CURRENTpt.
•Subsequent portal traversals will use the new MTD and return the new PID.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CAP
•SPC OBJ CURRENTptdid not refer to a PT Capability (CAP OBJ PT) or that capability had insu fficient
permissions.
34

5.4.5 Control Semaphore
Parameters:
status = ctrl_sm (SEL OBJsm, // Semaphore
UINT64stc) // Absolute Timeout
Flags:
00ZD
0 1 2 3
Description:
Prior to the hypercall:
•IfD=0 (Semaphore Up) :
–SPC OBJ CURRENTsmmust refer to an SM Capability (CAP OBJ SM) with permission CTRLUP.
•IfD=1 (Semaphore Down) :
–SPC OBJ CURRENTsmmust refer to an SM Capability (CAP OBJ SM) with permission CTRLDN.
If the hypercall completed successfully:
•IfD=0 (Semaphore Up) :
–If there were ECs blocked on the semaphore, then the microhypervisor has released one of those
blocked ECs. Otherwise, the microhypervisor has incremented the semaphore counter. The
timeout value and the Z-flag were ignored.
•IfD=1 (Semaphore Down) :
–If the semaphore counter was larger than zero, then the microhypervisor has decremented the
semaphore counter ( Z=0) or set it to zero ( Z=1). Otherwise, the microhypervisor has blocked
ECCURRENT on the semaphore. If the timeout value was non-zero, EC CURRENT unblocks with a
timeout status when the System Time Counter (STC) reaches or exceeds the specified value.
Blocking and releasing of ECs on a semaphore uses the FIFO queueing discipline.
Status:
SUCCESS
•The hypercall completed successfully.
TIMEOUT
•IfD=1: Down operation aborted when the timeout triggered.
ABORTED
•The SM designated by SPC OBJ CURRENTsmis being destructed concurrently.
OVRFLOW
•IfD=0: Up operation aborted because the semaphore counter would overflow.
BAD_CAP
•SPC OBJ CURRENTsmdid not refer to an SM Capability (CAP OBJ SM) or that capability had insu fficient
permissions.
BAD_CPU
•IfD=1on an interrupt semaphore: Attempt to wait for the interrupt on a di fferent CPU than the
CPU to which that interrupt has been routed via assign_int .
35

5.5 Platform Management
5.5.1 Control Hardware
Parameters:
status = ctrl_hw (UINT desc) // Descriptor
Flags:
OP
0 3
Description:
Modifies the platform hardware configuration or power management state.
Prior to the hypercall:
•The hypercall must be invoked by an Execution Context in the Root Protection Domain (PD ROOT ).
•IfOP=0 (S-State Transition) :
–The descriptor desc uses the following encoding:
– BAS
0 23 5 6 8 9 55
–The value Sdesignates the platform-wide reset or sleep state that shall be entered. The values
AandBare the first two bytes of the respective \_Sx package in the ACPI root namespace as
follows:
S A B Type Description
0x0 0x0 0x0 Reset Platform Reset
0x1 \_S1[0] \_S1[1]
ShallowS1: Stop Grant
0x2 \_S2[0] \_S2[1] S2: Power-On Suspend
0x3 \_S3[0] \_S3[1] S3: Suspend to RAM
0x4 \_S4[0] \_S4[1]DeepS4: Suspend to Disk
0x5 \_S5[0] \_S5[1] S5: Soft O ff
–The caller is responsible for invoking the necessary pre-sleep ACPI methods, for transitioning
platform devices into a suitable Dx sleep state, and for programming wakeup events.
•IfOP=4 (QOS Configuration) :
–The descriptor desc uses the following encoding:
–
L2
L3–
01 2 3 55
–Only Code and Data Prioritization (CDP) settings supported by the ambient CPU are valid.
*The L3 bit disables (0) or enables (1) CDP L3on the ambient CPU.
*The L2 bit disables (0) or enables (1) CDP L2on the ambient CPU.
–CAT/CDP or MBA settings cannot be configured for a CPU until a valid QOS configuration has
been established for that CPU. Subsequently, that QOS configuration cannot be changed anymore.
36

•IfOP=5 (CAT /CDP L3 Capacity Bitmask) :
–The descriptor desc uses the following encoding:
– L3 Capacity Bitmask N
0 15 16 4748 55
–N designates the CPU-local Class Of Service (COS) that shall be configured. Only COS below
COS L3of the ambient CPU are valid.
*If CDP L3is disabled on the ambient CPU:
· To configure the CAT L3Capacity Bitmask for a COS, use N=COS .
*If CDP L3is enabled on the ambient CPU:
· To configure the CDP L3-Data Capacity Bitmask for a COS, use N=(COS <<1).
· To configure the CDP L3-Code Capacity Bitmask for a COS, use N=(COS <<1)+1 .
–For the L3 Capacity Bitmask, all (and only) contiguous combinations of 1-bits up to the highest
capacity bit supported by the ambient CPU are valid.
•IfOP=6 (CAT /CDP L2 Capacity Bitmask) :
–The descriptor desc uses the following encoding:
– L2 Capacity Bitmask N
0 15 16 4748 55
–N designates the CPU-local Class Of Service (COS) that shall be configured. Only COS below
COS L2of the ambient CPU are valid.
*If CDP L2is disabled on the ambient CPU:
· To configure the CAT L2Capacity Bitmask for a COS, use N=COS .
*If CDP L2is enabled on the ambient CPU:
· To configure the CDP L2-Data Capacity Bitmask for a COS, use N=(COS <<1).
· To configure the CDP L2-Code Capacity Bitmask for a COS, use N=(COS <<1)+1 .
–For the L2 Capacity Bitmask, all (and only) contiguous combinations of 1-bits up to the highest
capacity bit supported by the ambient CPU are valid.
•IfOP=7 (MBA Delay) :
–The descriptor desc uses the following encoding:
– MBA Delay COS
0 15 16 31 32 55
–COS designates the CPU-local Class Of Service (COS) that shall be configured. Only COS below
COS MBof the ambient CPU are valid.
–For the MBA Delay, only values up to the highest delay supported by the ambient CPU are valid.
If the hypercall completed successfully:
•IfOP=0 (S-State Transition) :
–The platform resets or enters the specified ACPI sleep state.
–For reset or deep sleep states, the hypercall does not return.
–For shallow sleep states, the hypercall returns upon a wakeup event. The caller is responsible for
invoking the necessary post-sleep ACPI methods and for transitioning platform devices back into
the D0 working state.
•IfOP=4 (QOS Configuration) :
–The ambient CPU uses and locks down the QOS configuration.
•IfOP=5 (CAT /CDP L3 Capacity Bitmask) :
–The ambient CPU uses the L3 Capacity Bitmask for the designated COS.
•IfOP=6 (CAT /CDP L2 Capacity Bitmask) :
37

–The ambient CPU uses the L2 Capacity Bitmask for the designated COS.
•IfOP=7 (MBA Delay) :
–The ambient CPU uses the MBA Delay for the designated COS.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_HYP
•The hypercall was not invoked from the Root Protection Domain (PD ROOT ).
BAD_FTR
•The requested feature is not supported by the platform.
BAD_PAR
•IfOP=4: The QOS configuration is invalid.
•IfOP=5/6/7: The COS, CAT /CDP capacity bitmask or MBA delay is invalid.
•Otherwise: The requested operation (OP) is invalid.
ABORTED
•IfOP=0: A concurrent power management transition prevailed.
•IfOP=4: A QOS configuration has already been established for the ambient CPU.
•IfOP=5/6/7: A QOS configuration has not yet been established for the ambient CPU.
38

5.5.2 Assign Device
Parameters:
status = assign_dev (SEL OBJdc, // DMA Source Device
SEL OBJdma) // DMA Space
Flags:
0000
0 1 2 3
Description:
Assigns a DMA-capable device to the specified DMA Space (SPC DMA).
Prior to the hypercall:
•SPC OBJ CURRENTdcmust refer to a DC Capability (CAP OBJ DC) with permission ASSIGN DEV.
•SPC OBJ CURRENTdmamust refer to a DMA Space Capability (CAP OBJ DMA) with permission ASSIGN .
If the hypercall completed successfully:
•The device designated by SPC OBJ CURRENTdchas been assigned to the DMA Space designated by
SPC OBJ CURRENTdma.
•DMA transactions of that device will be managed by the SMMU and its resources that were encoded in
the Device Context during its creation. Prior users of those SMMU resources have been unconfigured.
Status:
SUCCESS
•The hypercall completed successfully.
TIMEOUT
•An SMMU command timed out at the hardware level.
ABORTED
•A concurrent assign_dev for the designated device prevailed.
BAD_CAP
•SPC OBJ CURRENTdcdid not refer to a DC Capability (CAP OBJ DC) or that capability had insu fficient
permissions.
•SPC OBJ CURRENTdmadid not refer to a DMA Space Capability (CAP OBJ DMA) or that capability had
insufficient permissions.
BAD_DEV
•The SMMU configured in the Device Context is invalid.
MEM_OBJ
•A translation table for the DMA Space or a mapping table for the SMMU could not be allocated.
39

5.5.3 Assign Interrupt
Parameters:
status = assign_int (SEL OBJsm, // Interrupt Semaphore (GSI)
SEL OBJdc, // MSI Source Device
UINT4cfg, // Interrupt Configuration
UINT16cpu, // Destination CPU Number
UINT16idx, // Table Index
UINTPTR&msi_addr, // OUT: MSI Address
UINTPTR&msi_data) // OUT: MSI Data
Flags:
000A
0 1 2 3
Description:
Attaches or Detaches an Interrupt Semaphore in a semaphore table slot and configures the associated GSI.
Prior to the hypercall:
•SPC OBJ CURRENTsmmust refer to an SM Capability (CAP OBJ SM) for an Interrupt Semaphore with
permission ASSIGN and thereby designates the GSI.
•The destination CPU must be valid ( cpu≤CPU MAX).
•The Semaphore (SM) table slot is determined as follows:
– Arm : The SM table is shared by all CPUs and indexed by GSI.
– x86 : The SM table is individual for each CPU, designated by cpuand indexed by idx.
•IfA=0 (Detach)
–The SM table slot must be attached to the Interrupt Semaphore designated by SPC OBJ CURRENTsm.
–SPC OBJ CURRENTdcmust refer to a Null Capability (CAP 0).
•IfA=1 (Attach)
–The SM table slot must be attached to the Interrupt Semaphore designated by SPC OBJ CURRENTsm
or must be detached.
–If the GSI type is PIN
*SPC OBJ CURRENTdcmust refer to a Null Capability (CAP 0).
–If the GSI type is MSI
*SPC OBJ CURRENTdcmust refer to a DC Capability (CAP OBJ DC) with permission ASSIGN INT.
If the hypercall completed successfully:
•IfA=0 (Detach)
–The Interrupt Semaphore designated by SPC OBJ CURRENTsmhas been detached from the SM table
slot and will no longer signal interrupt assertions arriving at that SM table slot.
•IfA=1 (Attach)
–The Interrupt Semaphore designated by SPC OBJ CURRENTsmhas been attached to the SM table slot
and will signal interrupt assertions arriving at that SM table slot.
–The GSI associated with that Interrupt Semaphore has been routed to the designated CPU and SM
table slot and configured according to cfg, which uses the following encoding:
OPTM
0 1 2 3Mask 0 =Unmasked 1 =Masked†
Trigger 0 =Edge-Triggered 1 =Level-Triggered†
Polarity 0 =Active-High 1 =Active-Low†
Ownership 0 =Host-Owned 1 =Guest-Owned‡
†Only valid for GSI type PIN
‡Only valid for the Arm architecture
40

–If the GSI type is PIN
*Any device that is connected to that interrupt pin is able to generate the interrupt.
*The returned msi_{addr,data} values are 0 and meaningless.
–If the GSI type is MSI
*Arm : GITS translates the device-specific EventID idxto the MSI.
*Only the device designated by SPC OBJ CURRENTdcis authorized to generate the MSI.
*The device driver must program the returned msi_{addr,data} values into the MSI registers
of that device to ensure proper interrupt operation.
Prior to the first invocation of assign_int for a GSI, the state of that interrupt is as follows:
•The interrupt is masked.
•The configuration (trigger, polarity, ownership) is undefined.
•The source device, destination CPU and semaphore table index are undefined.
Status:
SUCCESS
•The hypercall completed successfully.
BAD_CPU
•The specified CPU number was invalid.
BAD_CAP
•SPC OBJ CURRENTsmdid not refer to an SM Capability (CAP OBJ SM) for an Interrupt Semaphore or
that capability had insu fficient permissions.
•If Attach (PIN) or Detach: SPC OBJ CURRENTdcdid not refer to a Null Capability (CAP 0).
•If Attach (MSI): SPC OBJ CURRENTdcdid not refer to a DC Capability (CAP OBJ DC) or that capability
had insu fficient permissions.
BAD_DEV
•The Device Context (DC) configuration was invalid.
BAD_PAR
•The semaphore table index was invalid.
•The interrupt configuration was invalid.
ABORTED
•A concurrent operation destroyed the designated interrupt semaphore.
•The semaphore table slot was not in the required state.
41

6 Booting
6.1 NOVA Microhypervisor
6.1.1 NOVA Image
The bootloader must place all loadable ( PT_LOAD ) program segments of the NOV A microhypervisor into physical
memory (RAM) according to the physical addresses ( p_paddr ) and memory sizes ( p_memsz ) defined in the NOV A
microhypervisor ELF executable. The following is an example:
readelf -l nova.elf
Elf file type is EXEC (Executable file)
Entry point 0x404000
There are 2 program headers, starting at offset 64
Program Headers:
Type Offset VirtAddr PhysAddr
FileSiz MemSiz Flags Align
LOAD 0x00000000000000e8 0x0000000000404000 0x0000000000404000
0x000000000000091c 0x000000000000091c RW 0x8
LOAD 0x000000000000191c 0xffffffff8000491c 0x000000000040491c
0x0000000000014d4c 0x0000000000ffb6e4 RW 0x1000
If the physical memory range defined in the ELF executable is unsuitable for a particular platform, the bootloader
may shift all loadable program segments lower or higher in physical memory, by applying an o ffset, subject to the
following constraints:
•The same o ffset must be applied to each loadable program segment and to the entry point.
•The o ffset must comply with the load range and image alignment specified in the Multiboot2 [6] relocatable
header tag that is embedded in the NOV A microhypervisor ELF executable.
•The entire physical memory region occupied by the NOV A microhypervisor must be RAM.
After loading the NOV A microhypervisor into physical memory, the bootloader must invoke the entry point of the
ELF executable with architecture-specific preconditions (Arm, x86).
6.1.2 NOVA Integrity Measurement
The Reference Integrity Measurement (RIM) of the NOV A microhypervisor is printed during make install .
On hardware platforms with support for establishing a Dynamic Root of Trust for Measurement (DRTM), the
NOV A microhypervisor initiates a measured launch of itself. The Core Root of Trust for Measurement (CRTM)
extends a Launch Integrity Measurement (LIM) of the attestable region of the NOV A microhypervisor into PCR 17
of the Trusted Platform Module (TPM). The Launch Integrity Measurement is
•sensitive to (LIM ,RIM)
–any changes in the attestable region
–the bootloader passing command-line parameters to NOV A
•insensitive to (LIM =RIM)
–the hardware platform and NOV A patching its code to adapt to available hardware features
–the physical memory range into which NOV A has been loaded
–the memory map and other software modules that have been loaded
42

6.1.3 NOVA Spaces
6.1.3.1 NOVA Object Space
The NOV A Object Space (SPC OBJ NOV A) contains the following CAP OBJ:
SEL OBJ Capability Type Capability Resource Capability Permissions
2SBW OBJ-1 CAP OBJ SM Console Semaphore All defined permissions
2SBW OBJ-2 CAP OBJ OBJ NOV A Object Space TAKE
2SBW OBJ-3 CAP OBJ HST NOV A Host Space TAKE
2SBW OBJ-4 CAP OBJ PIO NOV A PIO Space†TAKE
2SBW OBJ-5 CAP OBJ MSR NOV A MSR Space†TAKE
2SBW OBJ-6 CAP OBJ OBJ Root Object Space All defined permissions
2SBW OBJ-7 CAP OBJ HST Root Host Space All defined permissions
2SBW OBJ-8 CAP OBJ PIO Root PIO Space†All defined permissions
0. . .CPUMAX CAP OBJ SC Idle Scheduling Contexts CTRL
Using SPC OBJ NOV A as source space for the ctrl_pd hypercall facilitates the delegation of CAP OBJfrom the
NOV A Object Space to another Object Space.
6.1.3.2 NOVA Host Space
In the NOV A Host Space (SPC HST NOV A), SEL HSTNrefers to
•CAP 0for memory protected by the NOV A microhypervisor (Arm, x86).
•CAP MEM for the 4 KiB page frame at physical addressN<<12with all defined permissions otherwise.
Using SPC HST NOV Aas source space for the ctrl_pd hypercall facilitates the delegation of CAP MEM from the
NOV A Host Space to another Host Space, Guest Space or DMA Space.
6.1.3.3 NOVA PIO Space
In the NOV A PIO Space (SPC PIO NOV A), SEL PIONrefers to
•CAP 0for an I /O port protected by the NOV A microhypervisor (x86).
•CAP PIOfor the I /O port number Nwith certain permissions otherwise (x86).
Using SPC PIO NOV A as source space for the ctrl_pd hypercall facilitates the delegation of CAP PIOfrom the
NOV A PIO Space to another PIO Space.
6.1.3.4 NOVA MSR Space
In the NOV A MSR Space (SPC MSR NOV A), SEL MSRNrefers to
•CAP 0for an MSR protected by the NOV A microhypervisor (x86).
•CAP MSRfor the MSR number Nwith certain permissions otherwise (x86).
Using SPC MSR NOV Aas source space for the ctrl_pd hypercall facilitates the delegation of CAP MSR from the
NOV A MSR Space to another MSR Space.
†Only on architectures, where this type of space exists, otherwise Null Capability (CAP 0).
43

6.2 Root Protection Domain
After the NOV A microhypervisor has initialized the system, it creates the following initial kernel objects:
•Root Protection Domain (PD ROOT )
–with Root Object Space (SPC OBJ ROOT)
–with Root Host Space (SPC HST ROOT)
–with Root PIO Space (SPC PIO ROOT)†
•Root Execution Context (EC ROOT ) on CPU BSP
–bound to SPC OBJ ROOT, SPC HST ROOT, SPC PIO ROOT†
–with SEL EVT=0
•Root Scheduling Context (SC ROOT ) on CPU BSP
–bound to EC ROOT
–with COS =0
–with Priority =highest priority
–with Budget =1000 ms
The Root Protection Domain is responsible for bootstrapping the other components of the user-mode framework
by creating additional kernel objects, loading additional images, assigning resources, etc.
6.2.1 Root Image
The image of the Root Protection Domain (PD ROOT ) must be a valid ELF executable ( ET_EXEC ) that has been
compiled for the respective hardware architecture and
•linked such that p_filesz = p_memsz
•loaded such that p_vaddr≡LOAD_ADDR∗+ p_offset (mod PAGE_SIZE)
holds for each loadable ( PT_LOAD ) program segment. These constraints ensure that the NOV A microhypervisor can
map all program segments directly from physical into virtual memory without any additional memory allocation
or copying. The following is an example:
readelf -l root.elf
Elf file type is EXEC (Executable file)
Entry point 0x10000120
There are 2 program headers, starting at offset 64
Program Headers:
Type Offset VirtAddr PhysAddr
FileSiz MemSiz Flags Align
LOAD 0x0000000000000000 0x0000000010000000 0x0000000010000000
0x0000000000000a75 0x0000000000000a75 R E 0x1000
LOAD 0x0000000000001000 0x0000000010001000 0x0000000010001000
0x000000000000f004 0x000000000000f004 RW 0x1000
†Only on architectures, where this type of space exists.
*The address in physical memory at which the bootloader has placed the ELF image.
44

6.2.2 Root Integrity Measurement
On hardware platforms with support for establishing a Dynamic Root of Trust for Measurement (DRTM), the
NOV A microhypervisor extends a Launch Integrity Measurement (LIM) of the attestable region of the Root
Protection Domain into PCR 19 of the Trusted Platform Module (TPM).
The attestable region is defined as the entire contents of the first loadable ( PT_LOAD ) program segment of the
PDROOT ELF image that is readable and /or executable, but not writable.
The attestable region of the Root Protection Domain should cover:
•the ELF header – to include the entry point of PD ROOT in the measurement
•the ELF program headers – to include the virtual-memory layout of PD ROOT in the measurement
•the code and read-only data sections of PD ROOT – to reflect their integrity
6.2.3 Root Spaces
Prior to the Root Execution Context (EC ROOT ) invoking the entry point of the Root Protection Domain (PD ROOT )
ELF image, the NOV A microhypervisor sets up the spaces for PD ROOT as described in the following subsections.
6.2.3.1 Root Object Space
The Root Object Space (SPC OBJ ROOT) contains the following initial CAP OBJ:
SEL OBJ Capability Type Capability Resource Capability Permissions
2SBW OBJ-1 CAP OBJ OBJ NOV A Object Space TAKE
2SBW OBJ-2 CAP OBJ OBJ Root Object Space All defined permissions
2SBW OBJ-3 CAP OBJ PD Root Protection Domain All defined permissions
2SBW OBJ-4 CAP OBJ EC Root Execution Context All defined permissions
2SBW OBJ-5 CAP OBJ SC Root Scheduling Context All defined permissions
All other SEL OBJin SPC OBJ ROOTinitially refer to a Null Capability (CAP 0).
6.2.3.2 Root Host Space
ELF Program Segments
The microhypervisor maps the Root Protection Domain (PD ROOT ) into the Root Host Space according
to the virtual addresses ( p_vaddr ), memory sizes ( p_memsz ) and page attributes ( p_flags ) of all
loadable ( PT_LOAD ) program segments defined in the PD ROOT ELF image.
Hypervisor Information Page
The microhypervisor maps the Hypervisor Information Page read-only into the Root Host Space 4 KiB
below the end of user-accessible virtual memory. The virtual address of the HIP is passed to EC ROOT at the
entry point (Arm, x86).
UTCB
The microhypervisor maps the User Thread Control Block of EC ROOT into the Root Host Space 4 KiB below
the address of the Hypervisor Information Page.
All other SEL HSTin SPC HST ROOTinitially refer to a Null Capability (CAP 0).
6.2.3.3 Root PIO Space
All SEL PIOin SPC PIO ROOTinitially refer to a Null Capability (CAP 0).
45

6.3 Hypervisor Information Page
The Hypervisor Information Page (HIP) is mapped in the Host Space of the Root Protection Domain and the initial
Stack Pointer (SP) of the Root Execution Context points to the HIP. Because the HIP is allocated and owned by
the microhypervisor, it cannot be delegated using ctrl_pd .
The HIP conveys information about the platform and configuration and has the following layout:
0 78 15 16 23 24 31 32 3940 4748 55 56 63+Length
Architecture-Dependenthhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
Architecture-Dependent+0xa0
Platform Features+0x98
SELGST/NOVA SELGST/ARCH SELHST/NOVA SELHST/ARCH +0x90
/ KIDMAX CPUMAX CPUBSP +0x88
/ MCOMSRMCOPIOMCODMAMCOGSTMCOHSTMCOOBJ+0x80
/ SBWMSRSBWPIOSBWDMASBWGSTSBWHSTSBWOBJ+0x78
STC Frequency+0x70
UEFI Desc Version UEFI Desc Size UEFI Memory Map Size+0x68
UEFI Memory Map Address+0x60
Framebuffer Resolution Y Framebuffer Resolution X+0x58
Framebuffer Pixel Pitch Framebuffer Pixel Format+0x50
Framebuffer Size+0x48
Framebuffer Address+0x40
ACPI RSDP Address+0x38
ROOT End Address+0x30
ROOT Start Address+0x28
MBUF End Address+0x20
MBUF Start Address+0x18
NOVA End Address+0x10
NOVA Start Address+0x08
Length Checksum Signature+0x00
0 78 15 16 23 24 31 32 3940 4748 55 56 63
All HIP fields are unsigned little-endian values, unless stated otherwise, and have the following meaning:
Signature
The value 0x41564f4e identifies the NOV A microhypervisor.
Checksum
The checksum is valid if 16bit-wise addition of the entire HIP contents produces a value of 0.
Length
Length of the entire HIP in bytes.
NOVA Start/End Address
Physical start and end address of the NOV A microhypervisor image.
MBUF Start/End Address
Physical start and end address of the memory bu ffer console region (see C.1).
ROOT Start/End Address
Physical start and end address of the root protection domain image.
46

ACPI RSDP Address
Physical address of the ACPI [7] Root System Description Pointer ( 0xffffffffffffffff if not present).
Framebuffer Address
Framebu ffer physical address.
Framebuffer Size
Framebu ffer size in bytes ( 0x0if not present).
Framebuffer Pixel Format
Pixel format, using the following encoding (bits not contained in any color bitmask are reserved):
RL, RH
Lowest and highest bit number of the bitmask representing color intensity for red.
GL, GH
Lowest and highest bit number of the bitmask representing color intensity for green.
BL, BH
Lowest and highest bit number of the bitmask representing color intensity for blue.
N−1
Number of bytes per pixel minus one: 0 =1B (8-bit), 1 =2B (16-bit), 2 =3B (24-bit), 3 =4B (32-bit)
N-1 BH BL GH GL RH RL
0 4 5 9 10 14 15 19 20 24 25 29 30 31
Framebuffer Pixel Pitch
Number of pixels per horizontal scanline.
Framebuffer Resolution X
Number of pixels in horizontal direction.
Framebuffer Resolution Y
Number of pixels in vertical direction.
UEFI Memory Map Address
Physical address of the UEFI [8] Memory Map ( 0xffffffffffffffff if not present).
UEFI Memory Map Size
Total size of the UEFI Memory Map ( 0if not present).
UEFI Desc Size
UEFI Memory Descriptor Size ( 0if not present).
UEFI Desc Version
UEFI Memory Descriptor Version ( 0if not present).
STC Frequency
Frequency of the System Time Counter (STC) in Hz.
SBW OBJ
Selector bit width: Object Capability Selector (SEL OBJ)
SBW HST
Selector bit width: Host Capability Selector (SEL HST)
SBW GST
Selector bit width: Guest Capability Selector (SEL GST)
SBW DMA
Selector bit width: DMA Capability Selector (SEL DMA)
SBW PIO
Selector bit width: PIO Capability Selector (SEL PIO)
SBW MSR
Selector bit width: MSR Capability Selector (SEL MSR)
MCO OBJ
Maximum contiguous order that avoids partial failures during capability updates in SPC OBJ.
47

MCO HST
Maximum contiguous order that avoids partial failures during capability updates in SPC HST.
MCO GST
Maximum contiguous order that avoids partial failures during capability updates in SPC GST.
MCO DMA
Maximum contiguous order that avoids partial failures during capability updates in SPC DMA.
MCO PIO
Maximum contiguous order that avoids partial failures during capability updates in SPC PIO.
MCO MSR
Maximum contiguous order that avoids partial failures during capability updates in SPC MSR.
CPU BSP
Bootstrap Processor. The CPU number on which EC ROOT and SC ROOT have been created.
CPU MAX
Maximum CPU number. The total number of CPUs is CPU MAX+1.
KID MAX
Maximum KID number. The total number of KIDs is KID MAX+1.
SEL HST/ARCH
Number of Capability Selectors required for handling architectual host events. (Arm, x86)
SEL HST/NOV A
Number of additional Capability Selectors required for handling microhypervisor host events. (Arm, x86)
SEL GST/ARCH
Number of Capability Selectors required for handling architectual guest events. (Arm, x86)
SEL GST/NOV A
Number of additional Capability Selectors required for handling microhypervisor guest events. (Arm, x86)
Platform Features
Supported platform features. (Arm, x86)
Architecture-Dependent
Architecture-dependent part. (Arm, x86)
48

Part IV
Application Binary Interface
49

7 ABI aarch64
7.1 Boot State
7.1.1 NOVA Microhypervisor
The bootloader must set up the CPU register state according to one of the launch types listed below when it transfers
control to the NOV A microhypervisor entry point. Furthermore, the following preconditions must be satisfied:
•The CPU must execute in EL2 (hypervisor mode) or in EL3 (monitor mode).
•Paging (MMU) must either be disabled ( SCTLR_ELx.M=0 ) or the page tables must provide an identity (1:1)
mapping with read /write/execute permissions.
•Interrupts must be disabled ( PSTATE.DAIF=0b1111 ).
•The physical memory region occupied by the microhypervisor image must be clean to the PoC.
•All DMA activity targeting the physical memory region occupied by the microhypervisor must be quiesced.
That physical memory region should also be protected against DMA accesses on systems with an SMMU.
7.1.1.1 Multiboot v2 Launch
Only this launch type supports 64-bit UEFI platforms.
Register Value /Description
IP Physical address of the NOV A ELF image entry point
X0 Multiboot v2 magic value ( 0x36d76289 ) [6]
X1 Physical address of the Multiboot v2 information structure [6]
Other /
The NOV A microhypervisor consumes the following multiboot tags, if present: 1,3,12,20.
7.1.1.2 Multiboot v1 Launch
Register Value /Description
IP Physical address of the NOV A ELF image entry point
X0 Multiboot v1 magic value ( 0x2badb002 ) [9]
X1 Physical address of the Multiboot v1 information structure [9]
Other /
The NOV A microhypervisor consumes the following multiboot flags, if present: 2,3.
7.1.1.3 Legacy Launch
Register Value /Description
IP Physical address of the NOV A ELF image entry point
X0 Physical address of the Flattened Device Tree (FDT) for the hardware platform†
X1 Physical address of the Root Protection Domain (PD ROOT ) ELF image
Other /
†Due to its alignment constraint, a valid FDT address will never be equal to a Multiboot magic value.
50

7.1.2 Root Protection Domain
The NOV A microhypervisor sets up the CPU register state as follows when it transfers control to the Root
Execution Context (EC ROOT ):
Register Value /Description
IP HV A of the Root Protection Domain (PD ROOT ) ELF image entry point
SP HV A of the Hypervisor Information Page (HIP)
X0X0at boot time†
X1X1at boot time†
X2X2at boot time†
Other /
7.2 Memory Map
The Root Protection Domain (PD ROOT ) can obtain a list of available /reserved physical memory regions as follows:
•On platforms using Unified Extensible Firmware Interface, by parsing the UEFI memory map.
•On platforms using Flattened Device Tree, by parsing the FDT.
7.3 Class Of Service
Class Of Service (COS) is currently not supported.
7.4 Protected Resources
Certain resources protected by the NOV A microhypervisor cannot be delegated and therefore remain inaccessible
to user-mode components. The following subsections enumerate these protected resources.
7.4.1 Physical Memory
The following physical memory regions are protected:
•NOV A microhypervisor – conveyed via HIP.
•GITS, GICD, GICR, GICC, GICH devices [10, 11] – conveyed via ACPI MADT or via FDT.
•SMMU devices [12, 13] – conveyed via ACPI IORT or via FDT.
•Firmware runtime services – conveyed via UEFI memory map.
†The register contains the preserved original value from the point when control was transferred from the bootloader to the microhypervisor.
51

7.5 Event-Specific Capability Selectors
For the delivery of exception /intercept messages, the microhypervisor performs an implicit portal traversal.
The selector for the destination portal (SEL OBJ):
•is determined by adding the exception /intercept number to the a ffected Execution Context’s Event Selector
Base (SEL EVT).
•indexes into the Object Space (SPC OBJ) of the a ffected EC’s Protection Domain (PD).
•must refer to a PT Capability (CAP OBJ PT) with permission EVENT that is bound to an EC on the same CPU
as the a ffected EC, otherwise the a ffected EC is killed.
7.5.1 Architectural Events
Host Exceptions and Guest Intercepts
SEL OBJ Exception /Intercept SEL OBJ Exception /Intercept
SEL EVT+ 0x00 Unknown Reason SEL EVT+ 0x20 Instruction Abort (lower EL)
SEL EVT+ 0x01 Trapped WFI or WFE SEL EVT+ 0x21 Instruction Abort (same EL) *
SEL EVT+ 0x02 reserved SEL EVT+ 0x22 PC Alignment Fault
SEL EVT+ 0x03 Trapped MCR or MRC SEL EVT+ 0x23 reserved
SEL EVT+ 0x04 Trapped MCRR or MRRC SEL EVT+ 0x24 Data Abort (lower EL)
SEL EVT+ 0x05 Trapped MCR or MRC SEL EVT+ 0x25 Data Abort (same EL) *
SEL EVT+ 0x06 Trapped LDC or STC SEL EVT+ 0x26 SP Alignment Fault
SEL EVT+ 0x07 SME, SVE, SIMD, FPU SEL EVT+ 0x27 Memory Operation Exception
SEL EVT+ 0x08 Trapped VMRS Access SEL EVT+ 0x28 Trapped FPU (AArch32)
SEL EVT+ 0x09 Trapped PAuth Instruction SEL EVT+ 0x29 reserved
SEL EVT+ 0x0a Trapped LD64B or ST64B SEL EVT+ 0x2a reserved
SEL EVT+ 0x0b reserved SEL EVT+ 0x2b reserved
SEL EVT+ 0x0c Trapped MRRC SEL EVT+ 0x2c Trapped FPU (AArch64)
SEL EVT+ 0x0d Branch Target Exception SEL EVT+ 0x2d reserved
SEL EVT+ 0x0e Illegal Execution State SEL EVT+ 0x2e reserved
SEL EVT+ 0x0f reserved SEL EVT+ 0x2f SError
SEL EVT+ 0x10 reserved SEL EVT+ 0x30 Breakpoint (lower EL)
SEL EVT+ 0x11 SVC (from AArch32 State) SEL EVT+ 0x31 Breakpoint (same EL) *
SEL EVT+ 0x12 HVC (from AArch32 State) SEL EVT+ 0x32 Software Step (lower EL)
SEL EVT+ 0x13 SMC (from AArch32 State) SEL EVT+ 0x33 Software Step (same EL) *
SEL EVT+ 0x14 reserved SEL EVT+ 0x34 Watchpoint (lower EL)
SEL EVT+ 0x15 SVC (from AArch64 State) * SEL EVT+ 0x35 Watchpoint (same EL) *
SEL EVT+ 0x16 HVC (from AArch64 State) SEL EVT+ 0x36 reserved
SEL EVT+ 0x17 SMC (from AArch64 State) SEL EVT+ 0x37 reserved
SEL EVT+ 0x18 Trapped MSR or MRS SEL EVT+ 0x38 BKPT (AArch32)
SEL EVT+ 0x19 Trapped SVE SEL EVT+ 0x39 reserved
SEL EVT+ 0x1a Trapped ERET SEL EVT+ 0x3a Vector Catch (AArch32)
SEL EVT+ 0x1b TSTART Exception SEL EVT+ 0x3b reserved
SEL EVT+ 0x1c PAuth Instruction Failure SEL EVT+ 0x3c BRK (AArch64)
SEL EVT+ 0x1d Trapped SME SEL EVT+ 0x3d reserved
SEL EVT+ 0x1e Granule Protection Exception SEL EVT+ 0x3e reserved
SEL EVT+ 0x1f reserved SEL EVT+ 0x3f reserved
Please refer to [3] for more details on each of these events.
*These events may be handled by the microhypervisor, in which case they will not cause portal traversals.
52

7.5.2 Microhypervisor Events
SEL OBJ Event
SEL EVT+ SELARCH+ 0x0 Startup
SEL EVT+ SELARCH+ 0x1 Recall
SEL EVT+ SELARCH+ 0x2 Virtual Timer
The value of SELARCHdepends on the origin of the event:
•SELARCH=SELHST/ARCH for events that occurred in the host.
•SELARCH=SELGST/ARCH for events that occurred in the guest.
53

7.6 Architecture-Dependent Structures
7.6.1 Platform Features
The supported platform features are defined as follows:
–
–
–
SMMU
0 1 2 3SMMU If set, then SMMU is active
7.6.2 Hypervisor Information Page
0 15 16 31 32 63+Length
/ CTXNUM SMGNUM Arch+0x08
LPINUM ESPINUM SPINUM Arch+0x00
0 15 16 31 32 63
SPI NUM
Total number of Shared Peripheral Interrupts (SPIs).
ESPI NUM
Total number of Extended Shared Peripheral Interrupts (ESPIs).
LPI NUM
Total number of Locality-Specific Peripheral Interrupts (LPIs).
SMG NUM
Total number of SMMUv2 Stream Mapping Groups (SMGs).
CTX NUM
Total number of SMMUv2 Translation Contexts (CTXs).
54

7.6.3 User Thread Control Block
– SEL_GST+0x2e0
– VMCR ELRSR+0x2d0
AP1R3 AP1R2 AP1R1 AP1R0+0x2c0
AP0R3 AP0R2 AP0R1 AP0R0+0x2b0
LR15 LR14+0x2a0
LR13 LR12+0x290
LR11 LR10+0x280
LR9 LR8+0x270
LR7 LR6+0x260
LR5 LR4+0x250
LR3 LR2+0x240
LR1 LR0+0x230GIC
CNTVOFF_EL2 CNTKCTL_EL1+0x220
CNTV_CTL_EL0 CNTV_CVAL_EL0+0x210
TMR
– HPFAR_EL2+0x200
FAR_EL2 ESR_EL2+0x1f0
SPSR_EL2 ELR_EL2+0x1e0
VMPIDR_EL2 VPIDR_EL2+0x1d0
HCRX_EL2 HCR_EL2+0x1c0EL2
– MDSCR_EL1+0x1b0
SCTLR_EL1 VBAR_EL1+0x1a0
AMAIR_EL1 MAIR_EL1+0x190
TCR_EL1 TTBR1_EL1+0x180
TTBR0_EL1 AFSR1_EL1+0x170
AFSR0_EL1 FAR_EL1+0x160
ESR_EL1 SPSR_EL1+0x150
ELR_EL1 CONTEXTIDR_EL1+0x140
TPIDR_EL1 SP_EL1+0x130EL1
– HSTR IFSR DACR+0x120
SPSR_und SPSR_irq SPSR_fiq SPSR_abt+0x110
A32
TPIDRRO_EL0 TPIDR_EL0+0x100
SP_EL0 X30 (LR_fiq)+0x0f0
X29 (SP_fiq) X28 (R12_fiq)+0x0e0
X27 (R11_fiq) X26 (R10_fiq)+0x0d0
X25 (R9_fiq) X24 (R8_fiq)+0x0c0
X23 (SP_und) X22 (LR_und)+0x0b0
X21 (SP_abt) X20 (LR_abt)+0x0a0
X19 (SP_svc) X18 (LR_svc)+0x090
X17 (SP_irq) X16 (LR_irq)+0x080
X15 (SP_hyp) X14 (LR_usr)+0x070
X13 (SP_usr) X12 (R12_usr)+0x060
X11 (R11_usr) X10 (R10_usr)+0x050
X9 (R9_usr) X8 (R8_usr)+0x040
X7 (R7) X6 (R6)+0x030
X5 (R5) X4 (R4)+0x020
X3 (R3) X2 (R2)+0x010
X1 (R1) X0 (R0)+0x000EL0
0 16 32 48 0 16 32 48
55

7.6.4 Message Transfer Descriptor
The Message Transfer Descriptor (MTD), which controls the subset of the architectural state transferred during
exceptions and intercepts, as described in Section 4.4.2, has the following layout:
SPACES
GIC
TMR
–
EL2_HPFAR
EL2_ESR_FAR
EL2_ELR_SPSR
EL2_IDR
EL2_HCR
–
EL1_MDSCR
EL1_SCTLR
EL1_VBAR
EL1_MAIR
EL1_TCR
EL1_TTBR
EL1_AFSR
EL1_ESR_FAR
EL1_ELR_SPSR
EL1_IDR
EL1_SP
–
A32_DIH
A32_SPSR
–
EL0_IDR
EL0_SP
FPR
GPR
ICI
POISON
0 1 2 3 4 5 7 8 10 11 12 13 14 15 16 17 18 19 20 23 24 25 26 27 29 30 31
Each MTD bit controls the transfer of the listed architectural state to /from the respective fields in the UTCB (7.6.3)
as follows:
•State with access rcan be read from the architectural state into the UTCB.
•State with access wcan be written from the UTCB into the architectural state.
MTD Bit Access Host Execution Context State Guest Execution Context State
POISON w Kills the EC HST Kills the EC GST
ICI†w Invalidates the entire I-Cache Invalidates the entire I-Cache
GPR rwX0. . .X30 X0. . .X30
EL0_SP rwSP_EL0 SP_EL0
EL0_IDR rwTPIDR_EL0, TPIDRRO_EL0 TPIDR_EL0, TPIDRRO_EL0
A32_SPSR rw– SPSR_ABT, SPSR_FIQ, SPSR_IRQ, SPSR_UND
A32_DIH rw– DACR, IFSR, HSTR
EL1_SP rw– SP_EL1
EL1_IDR rw– TPIDR_EL1, CONTEXTIDR_EL1
EL1_ELR_SPSR rw– ELR_EL1, SPSR_EL1
EL1_ESR_FAR rw– ESR_EL1, FAR_EL1
EL1_AFSR rw– AFSR0_EL1, AFSR1_EL1
EL1_TTBR rw– TTBR0_EL1, TTBR1_EL1
EL1_TCR rw– TCR_EL1
EL1_MAIR rw– MAIR_EL1, AMAIR_EL1
EL1_VBAR rw– VBAR_EL1
EL1_SCTLR rw– SCTLR_EL1
EL1_MDSCR rw– MDSCR_EL1
EL2_HCR rw– HCR_EL2, HCRX_EL2
EL2_IDR rw– VPIDR_EL2, VMPIDR_EL2
EL2_ELR_SPSR rwELR_EL2, SPSR_EL2∗ELR_EL2, SPSR_EL2
EL2_ESR_FAR rESR_EL2, FAR_EL2 ESR_EL2, FAR_EL2
EL2_HPFAR r– HPFAR_EL2
TMR rw–CNTV_CVAL_EL0, CNTV_CTL_EL0
CNTKCTL_EL1, CNTVOFF_EL2
GICrw–LR0 . . .LR15, APxR0 . . .APxR3
r ELRSR, VMCR
SPACES w– SEL_GST
*Only the condition flags are writable.
†Only a ffects a VIPT instruction cache of the local CPU. Has no e ffect on PIPT instruction caches, data caches, or caches of other CPUs.
56

7.6.5 Memory Attribute Descriptor
The Memory Attribute Descriptor (MAD) describes page attributes for a CAP MEM delegation from SPC HST NOV A.
– SHCA
0 2345 31
The fields are defined as follows:
CA
Specifies the cacheability attributes of the page according to the following table:
Encoding Cacheability Description
0x0 DEV Device
0x1 DEV_E Device, Early Ack
0x2 DEV_RE Device, Early Ack, Reordering
0x3 DEV_GRE Device, Early Ack, Reordering, Gathering
0x4 – reserved
0x5 MEM_NC Memory, Inner /Outer Non-Cacheable
0x6 MEM_WT Memory, Inner /Outer Write-Through
0x7 MEM_WB Memory, Inner /Outer Write-Back
SH
Specifies the shareability attributes of the page according to the following table:
Encoding Shareability Description
0x0 NONE Not Shareable
0x1 – reserved
0x2 OUTER Outer Shareable
0x3 INNER Inner Shareable
Please refer to [3] for the architectural behavior of these attributes.
7.6.6 Interrupt Semaphore Descriptor
The Interrupt Semaphore Descriptor (ISD) describes the interrupt associated with an Interrupt Semaphore.
– GSI
0 23 24 63
The fields are defined as follows:
GSI
Designates the interrupt via its GSI number (INTID) in one of the following ranges:
•32, 32 +SPI NUM– Shared Peripheral Interrupt (SPI)
•4096, 4096 +ESPI NUM– Extended Shared Peripheral Interrupt (ESPI)
•8192, 8192 +LPI NUM– Locality-Specific Peripheral Interrupt (LPI)
57

7.6.7 Device Topology Descriptor
The Device Topology Descriptor (DTD) describes the topology of a device as follows:
SID
Stream Identifier (SID) – if the device is subject to DMA translation.
SID Mask
Specifies which bits of that SID should be matched (0) or ignored (1) by the Stream Mapping Group.
SMG
Specifies the SMMU Stream Mapping Group (SMG) to use for that SID – must be <SMG NUM.
CTX
Specifies the SMMU Translation Context (CTX) to use for that SMG – must be <CTX NUM.
DID
Device Identifier (DID) – if the device is subject to MSI translation.
SMMU
Host-Physical Address (HPA) of the SMMU that performs DMA translation for the device.
GITS
Host-Physical Address (HPA) of the GITS that performs MSI translation for the device.
Reserved fields and fields that do not apply to a particular platform must be zero.
7.6.7.1 SMMUv3
0 11 12 63
GITS[63:12] –#2
SMMU[63:12] –#1
DID SID#0
0 31 32 63
7.6.7.2 SMMUv2
0 78 11 12 63
GITS[63:12] –CTX#2
SMMU[63:12] –SMG#1
DID SID Mask SID#0
0 15 16 31 32 63
System software must ensure an unambiguous assignment of Stream Identifiers to Stream Mapping Groups, i.e. it
must configure the SID /Mask fields across all Stream Mapping Groups such that no SID multi-matches can occur.
58

7.7 Calling Convention
The following pages describes the calling convention for each hypercall. A Host Execution Context calls into the
microhypervisor by loading the hypercall identifier and other parameters into the specified CPU registers and then
executes the svc #0 instruction [3].
The hypercall identifier consists of the hypercall number and hypercall-specific flags, as illustrated in Figure 7.1.
flags number
0 3 4 7
Figure 7.1: Hypercall Identifier
The status code returned from a hypercall has the format shown in Figure 7.2.
status
0 7
Figure 7.2: Status Code
The assignment of hypercall parameters to CPU registers is shown on the left side, the contents of the CPU registers
after the hypercall is shown on the right side.
IPC Call
pt[63−8]hypercall [7−0]X0ipc_call−−−−−−−−−−−−→ X0status [7−0]
mtd [31−0]X1 X1mtd [31−0]
–IP svc #0 IPIP+4
IPC Reply
hypercall [7−0]X0ipc_reply−−−−−−−−−−−−→ X0pid
mtd [31−0]X1 X1mtd [31−0]
–IP svc #0 IPPortal IP
Create Protection Domain
sel [63−8]hypercall [7−0]X0create_pd−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
–IP svc #0 IPIP+4
Create Execution Context
sel [63−8]hypercall [7−0]X0create_ec−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
hvp [63−12]X2 X2≡
evt [63−16]cpu [15−0]X3 X3≡
spX4 X4≡
–IP svc #0 IPIP+4
59

Create Scheduling Context
sel [63−8]hypercall [7−0]X0create_sc−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
ecX2 X2≡
scdX3 X3≡
–IP svc #0 IPIP+4
Create Portal
sel [63−8]hypercall [7−0]X0create_pt−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
ecX2 X2≡
ipX3 X3≡
–IP svc #0 IPIP+4
Create Semaphore
sel [63−8]hypercall [7−0]X0create_sm−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
valX2 X2≡
–IP svc #0 IPIP+4
Create Device Context
sel [63−8]hypercall [7−0]X0create_dc−−−−−−−−−−−−→ X0status [7−0]
pdX1 X1≡
dtd #0X2 X2≡
dtd #1X3 X3≡
dtd #2X4 X4≡
–IP svc #0 IPIP+4
Control Protection Domain
src [63−8]hypercall [7−0]X0ctrl_pd−−−−−−−−−−−−→ X0status [7−0]
dstX1 X1≡
ssb [63−12]ord [4−0]X2 X2≡
dsb [63−12]pmm [5−0]X3 X3≡
mad [31−0]X4 X4≡
–IP svc #0 IPIP+4
Control Execution Context
ec[63−8]hypercall [7−0]X0ctrl_ec−−−−−−−−−−−−→ X0status [7−0]
–IP svc #0 IPIP+4
Control Scheduling Context
sc[63−8]hypercall [7−0]X0ctrl_sc−−−−−−−−−−−−→ X0status [7−0]
–X1 X1stc
–IP svc #0 IPIP+4
60

Control Portal
pt[63−8]hypercall [7−0]X0ctrl_pt−−−−−−−−−−−−→ X0status [7−0]
pidX1 X1≡
mtd [31−0]X2 X2≡
–IP svc #0 IPIP+4
Control Semaphore
sm[63−8]hypercall [7−0]X0ctrl_sm−−−−−−−−−−−−→ X0status [7−0]
stcX1 X1≡
–IP svc #0 IPIP+4
Control Hardware
desc [63−8]hypercall [7−0]X0ctrl_hw−−−−−−−−−−−−→ X0status [7−0]
–IP svc #0 IPIP+4
Assign Device
dc[63−8]hypercall [7−0]X0assign_dev−−−−−−−−−−−−→ X0status [7−0]
dmaX1 X1≡
–IP svc #0 IPIP+4
Assign Interrupt
sm[63−8]hypercall [7−0]X0assign_int−−−−−−−−−−−−→ X0status [7−0]
dcX1 X1msi_addr
idx [47−32]cpu [31−16]cfg [3−0]X2 X2msi_data
–IP svc #0 IPIP+4
61

7.8 Supplementary Functionality
This section describes functions that do notconform to the calling convention for hypercalls. Because these
functions cannot perform capability-based access control, their invocation is restricted to the Root Protection
Domain (PD ROOT ). Invocation of these functions from any other Protection Domain generates an exception.
Secure Monitor Call
identifier [31−0]X0proxy_smc−−−−−−−−−−−−→ X0∼
–X1 X1∼
–X2 X2∼
–X3 X3∼
–X4 X4∼
–X5 X5∼
–X6 X6∼
–X7 X7∼
–X8 X8∼
–X9 X9∼
–X10 X10∼
–X11 X11∼
–X12 X12∼
–X13 X13∼
–X14 X14∼
–X15 X15∼
–X16 X16∼
–X17 X17∼
–IP svc #1 IPIP+4
This call is proxy-filtered by the microhypervisor. If the combination of invoked service (identifier [29−24]) and
function (identifier [15−0]) is listed in the table below, then the microhypervisor issues the corresponding SMC to
platform firmware on behalf of the caller. Otherwise, this function generates an exception. Register allocation
conforms to the Arm SMCCC [14].
Service Description Function Description
0x2 SIP Service Calls 0x0000–0xffff All functions
0x4 Standard Secure Service Calls0x0050–0x005f TRNG functions [15]
0x0130–0x013f PCI functions [16]
0x30–0x31 Trusted Application Calls 0x0000–0xffff All functions
0x32–0x3f Trusted OS Calls 0x0000–0xffff All functions
62

8 ABI x86-64
8.1 Boot State
8.1.1 NOVA Microhypervisor
The bootloader must set up the CPU register state according to one of the launch types listed below when it transfers
control to the NOV A microhypervisor entry point. Furthermore, the following preconditions must be satisfied:
•The CPU state must conform to a machine state defined in the Multiboot Specification v2 [6] or v1 [9].
•All DMA activity targeting the physical memory region occupied by the microhypervisor must be quiesced.
That physical memory region should also be protected against DMA accesses on systems with an IOMMU.
8.1.1.1 Multiboot v2 Launch
Only this launch type supports 64-bit UEFI platforms.
Register Value /Description
EIP Physical address of the NOV A ELF image entry point
EAX Multiboot v2 magic value ( 0x36d76289 ) [6]
EBX Physical address of the Multiboot v2 information structure [6]
Other /
The NOV A microhypervisor consumes the following multiboot tags, if present: 1,3,12,20.
8.1.1.2 Multiboot v1 Launch
Register Value /Description
EIP Physical address of the NOV A ELF image entry point
EAX Multiboot v1 magic value ( 0x2badb002 ) [9]
EBX Physical address of the Multiboot v1 information structure [9]
Other /
The NOV A microhypervisor consumes the following multiboot flags, if present: 2,3.
8.1.2 Root Protection Domain
The NOV A microhypervisor sets up the CPU register state as follows when it transfers control to the Root
Execution Context (EC ROOT ):
Register Value /Description
RIP HV A of the Root Protection Domain (PD ROOT ) ELF image entry point
RSP HV A of the Hypervisor Information Page (HIP)
RDIEAXat boot time†
RSIEBXat boot time†
Other /
†The register contains the preserved original value from the point when control was transferred from the bootloader to the microhypervisor.
63

8.2 Memory Map
The Root Protection Domain (PD ROOT ) can obtain a list of available /reserved physical memory regions as follows:
•On platforms using Multiboot v2 (UEFI boot services enabled), by parsing the UEFI memory map [8].
•On platforms using Multiboot v2, by parsing the Multiboot v2 memory map [6].
•On platforms using Multiboot v1, by parsing the Multiboot v1 memory map [9].
8.3 Class Of Service
Class Of Service (COS) support is indicated by CPUID leaf 0x7, sub-leaf 0x0:EBX [15].
The Root Protection Domain (PD ROOT ) must perform the following steps on each CPU to configure QOS settings:
1. Invoke ctrl_hw to establish a valid QOS configuration:
•CDP L3support is indicated by CPUID leaf 0x10 , sub-leaf 0x1:ECX [2]
•CDP L2support is indicated by CPUID leaf 0x10 , sub-leaf 0x2:ECX [2]
2. Determine COS NUM as the maximum of the following:
•COS L3from CPUID leaf 0x10 , sub-leaf 0x1:(1+EDX [15−0])>>X, where
–X=0 if CDP L3is disabled.
–X=1 if CDP L3is enabled.
•COS L2from CPUID leaf 0x10 , sub-leaf 0x2:(1+EDX [15−0])>>X, where
–X=0 if CDP L2is disabled.
–X=1 if CDP L2is enabled.
•COS MBfrom CPUID leaf 0x10 , sub-leaf 0x3:(1+EDX [15−0])
3. Invoke ctrl_hw to configure the following:
•For each COS L3: CAT /CDP L3 Capacity Bitmask(s)
•For each COS L2: CAT /CDP L2 Capacity Bitmask(s)
•For each COS MB: MBA Delay
8.4 Protected Resources
Certain resources protected by the NOV A microhypervisor cannot be delegated and therefore remain inaccessible
to user-mode components. The following subsections enumerate these protected resources.
8.4.1 Physical Memory
The following physical memory regions are protected:
•NOV A microhypervisor – conveyed via HIP.
•LAPIC, IOAPIC devices – conveyed via ACPI MADT.
•IOMMU devices [17, 18] – conveyed via ACPI DMAR or IVRS.
•DPR and TXT memory [19], except for TPM localities 0 and 1.
•MSI range – at 0xfee00000
•Firmware runtime services – conveyed via UEFI memory map.
64

8.4.2 I/O Ports
The CAP PIOcolumn in the table below details the permissions granted for I /O ports by the NOV A PIO Space.
I/O Port CAP PIO Exception or I /O Exit Enumeration VMM Handling
PM1a_CNT
CAP 0 Always ACPI FADT EmulatePM1b_CNT
PM2_CNT
SMI_CMD
Other A IfAis not set – Emulate or Passthrough
8.4.3 Model-Specific Registers
The CAP MSRcolumn in the table below details the permissions granted for MSRs by the NOV A MSR Space.
Model-Specific Register CAP MSR RDMSR Exit WRMSR Exit VMM Handling
IA32_SYSENTER_CS∗
RW IfRis not set IfWis not set
Emulate or PassthroughIA32_SYSENTER_ESP∗
IA32_SYSENTER_EIP∗
IA32_PAT∗
IA32_EFER∗
IA32_FS_BASE∗
IA32_GS_BASE∗
IA32_KERNEL_GS_BASE∗
IA32_PRED_CMDW Always IfWis not setIA32_FLUSH_CMD
IA32_TSC
R IfRis not set AlwaysIA32_PLATFORM_ID
IA32_BIOS_SIGN_ID
IA32_SGXLEPUBKEYHASH{0–3}∗
IA32_CORE_CAPABILITIES
IA32_MPERF
IA32_APERF
IA32_MTRR_CAP
IA32_ARCH_CAPABILITIES
IA32_MCU_OPT_CTRL
IA32_OVERCLOCKING_STATUS
IA32_PERF_STATUS
IA32_THERM_STATUS
IA32_PACKAGE_THERM_STATUS
IA32_SGX_SVN_STATUS
IA32_XSS∗
IA32_STAR∗
IA32_LSTAR∗
IA32_FMASK∗
IA32_TSC_AUX∗
Other CAP 0 Always Emulate
*The VMM can read /write the guest-e ffective MSR value via the UTCB.
65

8.5 Event-Specific Capability Selectors
For the delivery of exception /intercept messages, the microhypervisor performs an implicit portal traversal.
The selector for the destination portal (SEL OBJ):
•is determined by adding the exception /intercept number to the a ffected Execution Context’s Event Selector
Base (SEL EVT).
•indexes into the Object Space (SPC OBJ) of the a ffected EC’s Protection Domain (PD).
•must refer to a PT Capability (CAP OBJ PT) with permission EVENT that is bound to an EC on the same CPU
as the a ffected EC, otherwise the a ffected EC is killed.
8.5.1 Architectural Events
Host Exceptions
SEL OBJ Exception SEL OBJ Exception
SEL EVT+ 0x00 #DE SEL EVT+ 0x10 #MF
SEL EVT+ 0x01 #DB SEL EVT+ 0x11 #AC
SEL EVT+ 0x02 reserved SEL EVT+ 0x12 #MC *
SEL EVT+ 0x03 #BP SEL EVT+ 0x13 #XM
SEL EVT+ 0x04 #OF SEL EVT+ 0x14 #VE
SEL EVT+ 0x05 #BR SEL EVT+ 0x15 #CP
SEL EVT+ 0x06 #UD SEL EVT+ 0x16 reserved
SEL EVT+ 0x07 #NM * SEL EVT+ 0x17 reserved
SEL EVT+ 0x08 #DF * SEL EVT+ 0x18 reserved
SEL EVT+ 0x09 reserved SEL EVT+ 0x19 reserved
SEL EVT+ 0x0a #TS * SEL EVT+ 0x1a reserved
SEL EVT+ 0x0b #NP SEL EVT+ 0x1b reserved
SEL EVT+ 0x0c #SS SEL EVT+ 0x1c reserved
SEL EVT+ 0x0d #GP SEL EVT+ 0x1d reserved
SEL EVT+ 0x0e #PF SEL EVT+ 0x1e reserved
SEL EVT+ 0x0f reserved SEL EVT+ 0x1f reserved
*These events may be handled by the microhypervisor, in which case they will not cause portal traversals.
†These events may be force-enabled by the microhypervisor, in which case they will cause portal traversals.
66

Guest Intercepts (VMX)
SEL OBJ Intercept SEL OBJ Intercept
SEL EVT+ 0x00 Exception or NMI * SEL EVT+ 0x30 EPT Violation†
SEL EVT+ 0x01 External Interrupt * SEL EVT+ 0x31 EPT Misconfiguration
SEL EVT+ 0x02 Triple Fault†SEL EVT+ 0x32 INVEPT
SEL EVT+ 0x03 INIT†SEL EVT+ 0x33 RDTSCP
SEL EVT+ 0x04 SIPI†SEL EVT+ 0x34 Preemption Timer
SEL EVT+ 0x05 I/O SMI SEL EVT+ 0x35 INVVPID
SEL EVT+ 0x06 Other SMI SEL EVT+ 0x36 WBINVD, WBNOINVD
SEL EVT+ 0x07 Interrupt Window SEL EVT+ 0x37 XSETBV
SEL EVT+ 0x08 NMI Window SEL EVT+ 0x38 APIC Write
SEL EVT+ 0x09 Task Switch†SEL EVT+ 0x39 RDRAND
SEL EVT+ 0x0a CPUID†SEL EVT+ 0x3a INVPCID
SEL EVT+ 0x0b GETSEC†SEL EVT+ 0x3b VMFUNC
SEL EVT+ 0x0c HLT†SEL EVT+ 0x3c ENCLS
SEL EVT+ 0x0d INVD†SEL EVT+ 0x3d RDSEED
SEL EVT+ 0x0e INVLPG SEL EVT+ 0x3e PML Log Full
SEL EVT+ 0x0f RDPMC SEL EVT+ 0x3f XSA VES
SEL EVT+ 0x10 RDTSC SEL EVT+ 0x40 XRSTORS
SEL EVT+ 0x11 RSM SEL EVT+ 0x41 PCONFIG
SEL EVT+ 0x12 VMCALL SEL EVT+ 0x42 SPP Miss /Misconfiguration
SEL EVT+ 0x13 VMCLEAR SEL EVT+ 0x43 UMWAIT
SEL EVT+ 0x14 VMLAUNCH SEL EVT+ 0x44 TPAUSE
SEL EVT+ 0x15 VMPTRLD SEL EVT+ 0x45 LOADIWKEY
SEL EVT+ 0x16 VMPTRST SEL EVT+ 0x46 ENCLV
SEL EVT+ 0x17 VMREAD SEL EVT+ 0x47 SGX Conflict
SEL EVT+ 0x18 VMRESUME SEL EVT+ 0x48 ENQCMD PASID Failure
SEL EVT+ 0x19 VMWRITE SEL EVT+ 0x49 ENQCMDS PASID Failure
SEL EVT+ 0x1a VMXOFF SEL EVT+ 0x4a Bus Lock
SEL EVT+ 0x1b VMXON SEL EVT+ 0x4b Instruction Timeout
SEL EVT+ 0x1c CR Access * SEL EVT+ 0x4c SEAMCALL
SEL EVT+ 0x1d DR Access SEL EVT+ 0x4d TDCALL
SEL EVT+ 0x1e I/O Access†SEL EVT+ 0x4e RDMSRLIST
SEL EVT+ 0x1f RDMSR†SEL EVT+ 0x4f WRMSRLIST
SEL EVT+ 0x20 WRMSR†SEL EVT+ 0x50 URDMSR
SEL EVT+ 0x21 VM Entry Failure (State)†SEL EVT+ 0x51 UWRMSR
SEL EVT+ 0x22 VM Entry Failure (MSR) SEL EVT+ 0x52 reserved
SEL EVT+ 0x23 reserved SEL EVT+ 0x53 reserved
SEL EVT+ 0x24 MWAIT SEL EVT+ 0x54 reserved
SEL EVT+ 0x25 MTF SEL EVT+ 0x55 reserved
SEL EVT+ 0x26 reserved SEL EVT+ 0x56 reserved
SEL EVT+ 0x27 MONITOR SEL EVT+ 0x57 reserved
SEL EVT+ 0x28 PAUSE SEL EVT+ 0x58 reserved
SEL EVT+ 0x29 VM Entry Failure (MCE) SEL EVT+ 0x59 reserved
SEL EVT+ 0x2a reserved SEL EVT+ 0x5a reserved
SEL EVT+ 0x2b TPR Below Threshold SEL EVT+ 0x5b reserved
SEL EVT+ 0x2c APIC Access SEL EVT+ 0x5c reserved
SEL EVT+ 0x2d Virtualized EOI SEL EVT+ 0x5d reserved
SEL EVT+ 0x2e GDTR /IDTR Access SEL EVT+ 0x5e reserved
SEL EVT+ 0x2f LDTR /TR Access SEL EVT+ 0x5f reserved
Please refer to [4] for more details on each of these events.
67

8.5.2 Microhypervisor Events
SEL OBJ Event
SEL EVT+ SELARCH+ 0x0 Startup
SEL EVT+ SELARCH+ 0x1 Recall
The value of SELARCHdepends on the origin of the event:
•SELARCH=SELHST/ARCH for events that occurred in the host.
•SELARCH=SELGST/ARCH for events that occurred in the guest.
68

8.6 Architecture-Dependent Structures
8.6.1 Platform Features
The supported platform features are defined as follows:
–
SVM
VMX
IOMMU
0 1 2 3IOMMU If set, then IOMMU is active
VMX If set, then Intel VMX is active
SVM If set, then AMD SVM is active
8.6.2 Hypervisor Information Page
0 31 32 63+Length
Event Log Offset Event Log SizeArch+0x10
Event Log Physical AddressArch+0x08
GSINUM PINNUM / VECNUMArch+0x00
0 78 15 16 31 32 63
VEC NUM
Total number of per-CPU interrupt vectors.
PIN NUM
Total number of interrupts that are type PIN in GSI range0, PIN NUM. All other interrupts are type MSI.
GSI NUM
Total number of Global System Interrupts (GSIs).
Event Log Physical Address
Page-aligned physical address of the TPM event log [20].
Event Log Size
Size of the TPM event log in bytes.
Event Log Offset
Offset of the byte after the last TPM event log entry.
69

8.6.3 User Thread Control Block
SEL_MSR SEL_PIO+0x280
SEL_GST IA32_TSC_AUX+0x270
IA32_KERNEL_GS_BASE IA32_FMASK+0x260
IA32_LSTAR IA32_STAR+0x250
IA32_EFER IA32_PAT+0x240
IA32_SYSENTER_EIP IA32_SYSENTER_ESP+0x230
IA32_SYSENTER_CS IA32_APIC_BASE+0x220
IA32_SGXLEPUBKEYHASH3 IA32_SGXLEPUBKEYHASH2+0x210
IA32_SGXLEPUBKEYHASH1 IA32_SGXLEPUBKEYHASH0+0x200
IA32_XSS XCR0+0x1f0
DR7 CR8+0x1e0
CR4 CR3+0x1d0
CR2 CR0+0x1c0
PDPTE3 PDPTE2+0x1b0
PDPTE1 PDPTE0+0x1a0
Base IDTR Limit IDTR –+0x190
Base GDTR Limit GDTR –+0x180
Base LDTR Limit LDTR AR LDTR∗SEL LDTR+0x170
Base TR Limit TR AR TR∗SEL TR+0x160
Base GS Limit GS AR GS∗SEL GS+0x150
Base FS Limit FS AR FS∗SEL FS+0x140
Base ES Limit ES AR ES∗SEL ES+0x130
Base DS Limit DS AR DS∗SEL DS+0x120
Base SS Limit SS AR SS∗SEL SS+0x110
Base CS Limit CS AR CS∗SEL CS+0x100
IDT Vectoring Error IDT Vectoring Info Interruption Error Interruption Info†
+0x0f0
TPR Threshold PF Error Match PF Error Mask EXC Intercepts+0x0e0
CR4 Intercepts CR0 Intercepts+0x0d0
3rd Exec Controls 2nd Exec Controls 1st Exec Controls+0x0c0
– 3rd Qualification+0x0b0
2nd Qualification 1st Qualification+0x0a0
Activity Interruptibility Instruction Info Instruction Length+0x090
RIP RFLAGS+0x080
R15 R14+0x070
R13 R12+0x060
R11 R10+0x050
R9 R8+0x040
R7 (RDI) R6 (RSI)+0x030
R5 (RBP) R4 (RSP)+0x020
R3 (RBX) R2 (RDX)+0x010
R1 (RCX) R0 (RAX)+0x000
0 16 32 48 0 16 32 48
*See Section 8.6.3.1 for encoding details.
†See Section 8.6.3.2 for encoding details.
70

8.6.3.1 Encoding: Segment Access Rights
– UGD/BLAVLPDPL S Type
0 3 4 5 6 7 8 9 10 11 12
Field Description
U0=Segment Usable
1=Segment Unusable
G Granularity
D/B0=16-bit segment
1=32-bit segment
L 64-bit mode active (CS only)
AVL Available for use by system software
P Segment Present
DPL Descriptor Privilege Level
S0=System
1=Code or Data
Type Segment Type
8.6.3.2 Encoding: Interruption Information
V – NIEType Vector
0 7 8 10 11 12 13 31
Field Description
V0=FieldsE,Type ,Vector are invalid
1=FieldsE,Type ,Vector are valid
N0=Do not request an NMI window
1=Request an NMI window
I0=Do not request an interrupt window
1=Request an interrupt window
E0=Do not deliver the error code from the UTCB Interruption Error field
1=Deliver the error code from the UTCB Interruption Error field
Type0=External Interrupt
2=Non-Maskable Interrupt
3=Hardware Exception
4=Software Interrupt
5=Privileged Software Exception
6=Software Exception
7=Other Event (not delivered through IDT)
Vector IDT Vector of Interrupt or Exception
71

8.6.4 Message Transfer Descriptor
The Message Transfer Descriptor (MTD), which controls the subset of the architectural state transferred during
exceptions and intercepts, as described in Section 4.4.2, has the following layout:
SPACES
FPU
TLB
TSC
KERNEL_GS
EFER
PAT
SYSENTER
SYSCALL
APIC
SGX
XSAVE
DR
CR
PDPTE
IDTR
GDTR
LDTR
TR
FS/GS
DS/ES
CS/SS
INJ
TPR
CTRL
QUAL
STA
RIP
RFLAGS
GPR 8−15
GPR 0−7
POISON
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31
Each MTD bit controls the transfer of the listed architectural state to /from the respective fields in the UTCB (8.6.3)
as follows:
•State with access rcan be read from the architectural state into the UTCB.
•State with access wcan be written from the UTCB into the architectural state.
MTD Bit Access Host Execution Context State Guest Execution Context State
POISON w Kills the EC HST Kills the EC GST
GPR 0−7 rwR0. . .R7 R0. . .R7
GPR 8−15 rwR8. . .R15 R8. . .R15
RFLAGS rwRFLAGS∗RFLAGS
RIP rwRIP RIP, Instruction Length, Instruction Info
STA rw– Interruptibility State, Activity State
QUAL rQualifications†Qualifications‡
CTRL w–Execution Controls, CR Intercepts
EXC Intercepts, PF Error Mask/Match
TPR w– TPR Threshold
INJrw–Interruption Info, Interruption Error
r IDT Vectoring Info, IDT Vectoring Error
CS/SS rw– CS, SS (Selector, Base, Limit, AR)
DS/ES rw– DS, ES (Selector, Base, Limit, AR)
FS/GS rw– FS, GS (Selector, Base, Limit, AR)
TR rw– TR (Selector, Base, Limit, AR)
LDTR rw– LDTR (Selector, Base, Limit, AR)
GDTR rw– GDTR (Base, Limit)
IDTR rw– IDTR (Base, Limit)
PDPTE rw– PDPTE{0–3}
CR rw– CR0, CR2, CR3, CR4, CR8
DR rw– DR7
XSAVE rw– XCR0, IA32_XSS
SGX rw– IA32_SGXLEPUBKEYHASH{0–3}
APIC w– IA32_APIC_BASE§
SYSCALL rw– IA32_{STAR,LSTAR,FMASK}
SYSENTER rw– IA32_SYSENTER_{CS,ESP,EIP}
PAT rw– IA32_PAT
EFER rw– IA32_EFER
KERNEL_GS rw– IA32_KERNEL_GS_BASE
TSC rw– IA32_TSC_AUX
TLB w– Invalidates the TLB for the vCPU
SPACES w– SEL_GST, SEL_PIO, SEL_MSR
*Only the status and control flags are writable.
†Qualification fields contain exception error code (1st), page-fault linear address (2nd).
‡Qualification fields contain exit qualification (1st), guest-linear address (2nd), guest-physical address (3rd).
§APIC access page guest-physical address in the vCPU’s currently assigned guest space.
72

8.6.5 Memory Attribute Descriptor
The Memory Attribute Descriptor (MAD) describes page attributes for a CAP MEM delegation from SPC HST NOV A.
– KID CA
0 23 1718 31
The fields are defined as follows:
CA
Specifies the cacheability attributes of the page according to the following table:
Encoding Cacheability Description
0x0 WB Write Back
0x1 WT Write Through
0x2 WC Write Combining
0x3 UC Strong Uncacheable
0x4 WP Write Protected
0x5 – reserved
0x6 – reserved
0x7 – reserved
KID
Specifies the key identifier of the cryptographic key to be used for memory encryption – must be ≤KID MAX.
Please refer to [4, 5] for the architectural behavior of these attributes.
8.6.6 Interrupt Semaphore Descriptor
The Interrupt Semaphore Descriptor (ISD) describes a GSI associated with an Interrupt Semaphore.
– S GSI
0 15 16 31 32 63
The fields are defined as follows:
S
PCI Segment
GSI
Designates the interrupt via its GSI number in the following range:
•0, GSI NUM– Global System Interrupt (GSI)
8.6.7 Device Topology Descriptor
The Device Topology Descriptor (DTD) describes the topology of a device as follows:
S, B, D, F
PCI Segment /Bus/Device /Function (SBDF).
SMMU
Host-Physical Address (HPA) of the SMMU that performs DMA and MSI translation for the device.
Reserved fields and fields that do not apply to a particular platform must be zero.
0 11 12 63
–#2
SMMU[63:12] –#1
– S B DF#0
0 23 78 15 16 31 32 63
73

8.7 Calling Convention
The following pages describes the calling convention for each hypercall. A Host Execution Context calls into the
microhypervisor by loading the hypercall identifier and other parameters into the specified CPU registers and then
executes the syscall instruction [4, 5].
The hypercall identifier consists of the hypercall number and hypercall-specific flags, as illustrated in Figure 8.1.
flags number
0 3 4 7
Figure 8.1: Hypercall Identifier
The status code returned from a hypercall has the format shown in Figure 8.2.
status
0 7
Figure 8.2: Status Code
The assignment of hypercall parameters to CPU registers is shown on the left side, the contents of the CPU registers
after the hypercall is shown on the right side.
IPC Call
pt[63−8]hypercall [7−0]RDIipc_call−−−−−−−−−−−−→ RDIstatus [7−0]
mtd [31−0]RSI RSImtd [31−0]
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
IPC Reply
hypercall [7−0]RDIipc_reply−−−−−−−−−−−−→ RDIpid
mtd [31−0]RSI RSImtd [31−0]
–RCX RCXPortal IP
–R11 R110x202
–RIP syscall RIPPortal IP
Create Protection Domain
sel [63−8]hypercall [7−0]RDIcreate_pd−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
74

Create Execution Context
sel [63−8]hypercall [7−0]RDIcreate_ec−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
hvp [63−12]RDX RDX≡
evt [63−16]cpu [15−0]RAX RAX≡
spR8 R8≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Create Scheduling Context
sel [63−8]hypercall [7−0]RDIcreate_sc−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
ecRDX RDX≡
scdRAX RAX≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Create Portal
sel [63−8]hypercall [7−0]RDIcreate_pt−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
ecRDX RDX≡
ipRAX RAX≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Create Semaphore
sel [63−8]hypercall [7−0]RDIcreate_sm−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
valRDX RDX≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Create Device Context
sel [63−8]hypercall [7−0]RDIcreate_dc−−−−−−−−−−−−→ RDIstatus [7−0]
pdRSI RSI≡
dtd #0RDX RDX≡
dtd #1RAX RAX≡
dtd #2R8 R8≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
75

Control Protection Domain
src [63−8]hypercall [7−0]RDIctrl_pd−−−−−−−−−−−−→ RDIstatus [7−0]
dstRSI RSI≡
ssb [63−12]ord [4−0]RDX RDX≡
dsb [63−12]pmm [5−0]RAX RAX≡
mad [31−0]R8 R8≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Control Execution Context
ec[63−8]hypercall [7−0]RDIctrl_ec−−−−−−−−−−−−→ RDIstatus [7−0]
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Control Scheduling Context
sc[63−8]hypercall [7−0]RDIctrl_sc−−−−−−−−−−−−→ RDIstatus [7−0]
–RSI RSIstc
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Control Portal
pt[63−8]hypercall [7−0]RDIctrl_pt−−−−−−−−−−−−→ RDIstatus [7−0]
pidRSI RSI≡
mtd [31−0]RDX RDX≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Control Semaphore
sm[63−8]hypercall [7−0]RDIctrl_sm−−−−−−−−−−−−→ RDIstatus [7−0]
stcRSI RSI≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Control Hardware
desc [63−8]hypercall [7−0]RDIctrl_hw−−−−−−−−−−−−→ RDIstatus [7−0]
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
76

Assign Device
dc[63−8]hypercall [7−0]RDIassign_dev−−−−−−−−−−−−→ RDIstatus [7−0]
dmaRSI RSI≡
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
Assign Interrupt
sm[63−8]hypercall [7−0]RDIassign_int−−−−−−−−−−−−→ RDIstatus [7−0]
dcRSI RSImsi_addr
idx [47−32]cpu [31−16]cfg [3−0]RDX RDXmsi_data
–RCX RCXRIP+2
–R11 R110x202
–RIP syscall RIPRIP+2
77

Part V
Appendix
78

A Acronyms
ACPI Advanced Configuration and Power Interface [7]
BSP Bootstrap Processor
CAP Capability
CAP 0 Null Capability
CAP OBJ Object Capability
CAP OBJ OBJ Object Space Capability
CAP OBJ HST Host Space Capability
CAP OBJ GST Guest Space Capability
CAP OBJ DMA DMA Space Capability
CAP OBJ PIO PIO Space Capability
CAP OBJ MSR MSR Space Capability
CAP OBJ PD PD Capability
CAP OBJ EC EC Capability
CAP OBJ SC SC Capability
CAP OBJ PT PT Capability
CAP OBJ SM SM Capability
CAP OBJ DC DC Capability
CAP MEM Memory Capability
CAP PIO PIO Capability
CAP MSR MSR Capability
CAT Cache Allocation Technology [4]
CDP Code and Data Prioritization [4]
COS Class Of Service [Arm, x86] [4]
CPU Central Processing Unit [3, 4, 5]
CRTM Core Root of Trust for Measurement [21, 22]
CTX Translation Context [12, 13]
DID Device Identifier [11]
DMA Direct Memory Access
DPR DMA Protected Range
DRTM Dynamic Root of Trust for Measurement [21, 22]
DTD Device Topology Descriptor [Arm, x86]
DC Device Context
DV A DMA-Virtual Address
EC Execution Context
EC HST Host Execution Context
EC GST Guest Execution Context
EC CURRENT Current Execution Context
79

EC ROOT Root Execution Context
ELF Executable and Linkable Format [23]
ESPI Extended Shared Peripheral Interrupt [10, 11]
FDT Flattened Device Tree [24]
FPU Floating Point Unit [3, 4, 5]
GIC Generic Interrupt Controller [10, 11]
GICC GIC CPU Interface
GICD GIC Distributor
GICH GIC HYP Interface
GICR GIC Redistributor
GITS GIC Interrupt Translation Service
GPA Guest-Physical Address
GSI Global System Interrupt [7]
HIP Hypervisor Information Page [Arm, x86]
HPA Host-Physical Address
HV A Host-Virtual Address
IOAPIC I/O Advanced Programmable Interrupt Controller
IOMMU I/O Memory Management Unit [17, 18]
IP Instruction Pointer
IPC Inter-Process Communication
ISD Interrupt Semaphore Descriptor [Arm, x86]
KID Key Identifier
LAPIC Local Advanced Programmable Interrupt Controller
LIM Launch Integrity Measurement
LPI Locality-Specific Peripheral Interrupt [10, 11]
MAD Memory Attribute Descriptor [Arm, x86]
MBA Memory Bandwidth Allocation [4]
MMU Memory Management Unit [3, 4, 5]
MSI Message-Signaled Interrupt [25, 26]
MSR Model-Specific Register [4, 5]
MTD Message Transfer Descriptor [Arm, x86]
NOV A N OV A OSVirtualization Architecture [2]
PCI Peripheral Component Interconnect [25, 26]
PCR Platform Configuration Register [21, 22]
PIO Port-Based I /O
PD Protection Domain
PD ROOT Root Protection Domain
PID Portal Identifier
PT Portal
QOS Quality Of Service
RIM Reference Integrity Measurement
SBDF PCI Segment /Bus/Device /Function
80

SC Scheduling Context
SCCURRENT Current Scheduling Context
SCROOT Root Scheduling Context
SCD Scheduling Context Descriptor
SEL Capability Selector
SEL EVT Event Selector Base [Arm, x86]
SEL OBJ Object Capability Selector
SEL HST Host Capability Selector
SEL GST Guest Capability Selector
SEL DMA DMA Capability Selector
SEL PIO PIO Capability Selector
SEL MSR MSR Capability Selector
SID Stream Identifier [12, 13]
SM Semaphore
SMG Stream Mapping Group [12, 13]
SMMU System Memory Management Unit [12, 13]
SP Stack Pointer
SPC OBJ Object Space
SPC HST Host Space
SPC GST Guest Space
SPC DMA DMA Space
SPC PIO PIO Space
SPC MSR MSR Space
SPC OBJ CURRENT Current Object Space
SPC OBJ NOV A NOV A Object Space
SPC HST NOV A NOV A Host Space
SPC PIO NOV A NOV A PIO Space
SPC MSR NOV A NOV A MSR Space
SPC OBJ ROOT Root Object Space
SPC HST ROOT Root Host Space
SPC PIO ROOT Root PIO Space
SPI Shared Peripheral Interrupt [10, 11]
STC System Time Counter
TPM Trusted Platform Module [21, 22]
TXT Trusted Execution Technology [19]
UART Universal Asynchronous Receiver Transmitter
UEFI Unified Extensible Firmware Interface [8]
UTCB User Thread Control Block [Arm, x86]
VMM Virtual-Machine Monitor
81

ipc_call Hypercall [Arm, x86]: IPC Call
ipc_reply Hypercall [Arm, x86]: IPC Reply
create_pd Hypercall [Arm, x86]: Create Protection Domain
create_ec Hypercall [Arm, x86]: Create Execution Context
create_sc Hypercall [Arm, x86]: Create Scheduling Context
create_pt Hypercall [Arm, x86]: Create Portal
create_sm Hypercall [Arm, x86]: Create Semaphore
create_dc Hypercall [Arm, x86]: Create Device Context
ctrl_pd Hypercall [Arm, x86]: Control Protection Domain
ctrl_ec Hypercall [Arm, x86]: Control Execution Context
ctrl_sc Hypercall [Arm, x86]: Control Scheduling Context
ctrl_pt Hypercall [Arm, x86]: Control Portal
ctrl_sm Hypercall [Arm, x86]: Control Semaphore
ctrl_hw Hypercall [Arm, x86]: Control Hardware
assign_dev Hypercall [Arm, x86]: Assign Device
assign_int Hypercall [Arm, x86]: Assign Interrupt
82

B Bibliography
[1]RFC 2119 . Internet Engineering Task Force (IETF), 1997. URL https://tools.ietf.org/html/
rfc2119 . v
[2] Udo Steinberg and Bernhard Kauer. NOV A: A Microhypervisor-Based Secure Virtualization Architecture. In
Proceedings of the 5th ACM SIGOPS /EuroSys European Conference on Computer Systems , pages 209–222.
ACM, 2010. ISBN 978-1-60558-577-2. URL https://doi.acm.org/10.1145/1755913.1755935 . 2,
80
[3]Arm Architecture Reference Manual ARMv8, for ARMv8-A Architecture Profile . Arm Limited, 2025. URL
https://developer.arm.com/documentation/ddi0487/ . Document Number: DDI0487L.b. 8, 52, 57,
59, 79, 80
[4]Intel 64 and IA-32 Architectures Software Developer’s Manual, Combined Volumes: 1, 2A, 2B, 2C, 2D, 3A,
3B, 3C, 3D, and 4 . Intel Corporation, 2025. URL https://software.intel.com/en-us/articles/
intel-sdm . Document Number: 325462-088. 8, 67, 73, 74, 79, 80
[5]AMD64 Architecture Programmer’s Manual: Volumes 1–5 . Advanced Micro Devices, Inc., 2024. URL
https://developer.amd.com/resources/developer-guides-manuals . Document Number: 40332.
8, 73, 74, 79, 80
[6] Yoshinori K. Okuji, Bryan Ford, Erich Stefan Boleyn, Kunihiro Ishiguro, Vladimir Serbinenko, and Daniel
Kiper. The Multiboot2 Specification , 2016. URL https://www.gnu.org/software/grub/manual/
multiboot2/multiboot.pdf . Version 2.0. 42, 50, 63, 64
[7]Advanced Configuration and Power Interface (ACPI) Specification . UEFI Forum, Inc., 2025. URL https:
//uefi.org/specifications . Version 6.6. 47, 79, 80
[8]Unified Extensible Firmware Interface (UEFI) Specification . UEFI Forum, Inc., 2024. URL https://uefi.
org/specifications . Version 2.11. 47, 64, 81
[9] Yoshinori K. Okuji, Bryan Ford, Erich Stefan Boleyn, and Kunihiro Ishiguro. The Multiboot Specification ,
2010. URL https://www.gnu.org/software/grub/manual/multiboot/multiboot.pdf . Version
0.6.96. 50, 63, 64
[10] Arm Generic Interrupt Controller Architecture Specification Version 2 . Arm Limited, 2013. URL https:
//developer.arm.com/documentation/ihi0048/ . Document Number: IHI0048B.b. 51, 80, 81
[11] Arm Generic Interrupt Controller Architecture Specification Version 3 and Version 4 . Arm Limited, 2024.
URLhttps://developer.arm.com/documentation/ihi0069/ . Document Number: IHI0069H.b. 51,
79, 80, 81
[12] Arm System Memory Management Unit Architecture Specification Version 2 . Arm Limited, 2016. URL
https://developer.arm.com/documentation/ihi0062/ . Document Number: IHI0062D.c. 51, 79, 81
[13] Arm System Memory Management Unit Architecture Specification Version 3 . Arm Limited, 2025. URL
https://developer.arm.com/documentation/ihi0070/ . Document Number: IHI0070G.b. 51, 79, 81
[14] Arm SMC Calling Convention . Arm Limited, 2025. URL https://developer.arm.com/
documentation/den0028/ . Document Number: DEN0028G. 62
[15] Arm True Random Number Generator Firmware Interface . Arm Limited, 2022. URL https://developer.
arm.com/documentation/den0098/ . Document Number: DEN0098. 62
[16] Arm PCI Configuration Space Access Firmware Interface . Arm Limited, 2022. URL https://developer.
arm.com/documentation/den0115/ . Document Number: DEN0115. 62
83

[17] Intel Virtualization Technology for Directed I /O Architecture Specification . Intel Corpora-
tion, 2024. URL https://www.intel.com/content/www/us/en/develop/download/
intel-virtualization-technology-for-directed-io-architecture-specification.html .
Document Number: D51397-017, Revision 5.0. 64, 80
[18] AMD I /O Virtualization Technology (IOMMU) Specification . Advanced Micro
Devices, Inc., 2023. URL https://www.amd.com/en/support/tech-docs/
amd-io-virtualization-technology-iommu-specification . Document Number: 48882. 64,
80
[19] Intel Trusted Execution Technology (Intel TXT) Software Development Guide . Intel Corporation, 2025.
Document Number: 315168-017, Revision 17.6. 64, 81
[20] TCG PC Client Platform, Firmware Profile Specification . Trusted Computing Group, 2023. URL https:
//trustedcomputinggroup.org/resources . Version 1.06, Revision 52. 69
[21] TCG PC Client Platform, TPM Profile Specification for TPM 2.0 . Trusted Computing Group, 2020. URL
https://trustedcomputinggroup.org/resources . Version 1.05, Revision 14. 79, 80, 81
[22] TCG Trusted Platform Module Library Specification, Family "2.0" . Trusted Computing Group, 2024. URL
https://trustedcomputinggroup.org/resources . Revision 1.83. 79, 80, 81
[23] ELF Object File Format . Xinuos, Inc., 2025. URL https://gabi.xinuos.com/ . Version 4.3. 80
[24] Devicetree Specification . Linaro Limited, 2023. URL https://www.devicetree.org/specifications .
Version 0.4. 80
[25] PCI Local Bus Specification . PCI-SIG, 2004. URL https://pcisig.com/specifications . Revision
3.0. 80
[26] PCI Express Base Specification . PCI-SIG, 2025. URL https://pcisig.com/specifications . Revision
7.0. 80
84

C Console
C.1 Memory-Buffer Console
The NOV A microhypervisor implements a memory-bu ffer console that provides boot-time and run-time debug
output. The memory-bu ffer console consists of a signaling semaphore (see 6.1.3.1) and an in-memory data
structure with a header and a bu ffer as follows:
0 78 15 16 23 24 31 32 3940 4748 55 56 63End
Char N-1 Char N-2 Char N-3 Char N-4 Char N-5 Char N-6 Char N-7 Char N-8hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
Char 15 Char 14 Char 13 Char 12 Char 11 Char 10 Char 9 Char 8Start+0x10
Char 7 Char 6 Char 5 Char 4 Char 3 Char 2 Char 1 Char 0Start+0x08Buffer
– WrIdx – RdIdxStart+0x00}Header
0 31 32 63
The start address and end address of the memory-bu ffer console are conveyed in the HIP.
The bu ffer size (Ncharacters) can be computed as:
N = MBUF End Address - MBUF Start Address - MBUF Header Size
The fields of the header are used as follows:
•RdIdx is in range0,N-1.
It points to the next character in the bu ffer that the console consumer will read and is typically advanced by
the console consumer.
•WrIdx is in range0,N-1.
It points to the next character in the bu ffer that the NOV A microhypervisor will write and is only advanced
by the NOV A microhypervisor.
•The bu ffer is empty if RdIdx is equal to WrIdx .
•Otherwise WrIdx is ahead of RdIdx , wrapping around the bu ffer sizeNaccordingly, i.e. character N+xwill
be stored in the same bu ffer slot as character x.
•If the bu ffer becomes full, the NOV A microhypervisor advances RdIdx , forcing the oldest character to be
discarded from the bu ffer.
•At the end of each line, the NOV A microhypervisor invokes ctrl_sm (Up) on the signaling semaphore. The
console consumer should use ctrl_sm (Down ) on the signaling semaphore instead of polling WrIdx .
Note
The memory-bu ffer console remains always enabled. The Root Protection Domain can delegate the data structure to any Protection Domain,
which can subsequently act as console consumer, read the debug output from the bu ffer, and write it to a suitable output device.
85

C.2 Framebuffer Console
The framebu ffer console requires a UEFI platform with Graphics Output Protocol and an EDID-capable display.
It provides boot-time-only debug output of the microhypervisor until ownership of platform hardware transitions
to the Root Protection Domain.
Note
The framebu ffer console is enabled by default. It can be disabled by specifying the nofbuf parameter on the microhypervisor command line.
Disabling the framebu ffer console can slightly improve the time it takes to boot the system.
C.3 UART Consoles
The UART consoles require a platform with an RS232 port /header connected to another system via a serial cable.
They provide boot-time-only debug output of the microhypervisor until ownership of platform hardware transitions
to the Root Protection Domain. The receiver terminal must be configured for 115200 baud and8N1mode.
Note
All UART consoles are enabled by default. They can be disabled by specifying the nouart parameter on the microhypervisor command line.
Disabling the UART consoles can significantly improve the time it takes to boot the system.
86

D Download
The source code of the NOV A microhypervisor and the latest version of this document can be downloaded from
GitHub:https://github.com/udosteinberg/NOVA
87


```
