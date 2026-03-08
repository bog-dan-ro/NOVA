# conf/archs/aarch64.cmake — AArch64 architecture configuration

set(NOVA_ARCH_FLAGS
    -march=armv8-a
    -mcmodel=large
    -mgeneral-regs-only
    -mno-outline-atomics
    -mstrict-align
)

# Bootstrap-specific flags (no -mcmodel=large for relocability)
set(NOVA_BSP_ARCH_FLAGS
    -march=armv8-a
    -mgeneral-regs-only
    -mno-outline-atomics
    -mstrict-align
)

set(NOVA_ARCH_DEFINES "BOARD_${NOVA_BOARD}")
set(NOVA_ARCH_WFLAGS  -Wpedantic)
set(NOVA_ARCH_LFLAGS  "")
set(NOVA_OUTPUT_NAME  "${NOVA_ARCH}-${NOVA_BOARD}-nova")
