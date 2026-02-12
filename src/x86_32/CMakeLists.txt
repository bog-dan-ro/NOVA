# src/x86_32/CMakeLists.txt — x86_32 architecture sources

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_uart_sio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/counter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ec_arch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ec_exc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/entry.S
    ${CMAKE_CURRENT_SOURCE_DIR}/fpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gdt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hip_arch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/idt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/integrity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libgcc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/patch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptab_hpt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/regs.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_hst.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_pio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/start.S
    ${CMAKE_CURRENT_SOURCE_DIR}/tlb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tss.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utcb_arch.cpp
)
