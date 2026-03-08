# conf/archs/riscv64.cmake — RISC-V 64-bit architecture configuration

set(NOVA_ARCH_FLAGS
    -march=rv64imafdc
    -mabi=lp64d
    -mcmodel=medany
    -mno-relax
)

set(NOVA_BSP_ARCH_FLAGS
    -march=rv64imafdc
    -mabi=lp64d
    -mcmodel=medany
    -mno-relax
)

set(NOVA_ARCH_DEFINES "BOARD_${NOVA_BOARD}")
set(NOVA_ARCH_WFLAGS  -Wpedantic)
set(NOVA_ARCH_LFLAGS  "")
set(NOVA_OUTPUT_NAME  "${NOVA_ARCH}-${NOVA_BOARD}-nova")
