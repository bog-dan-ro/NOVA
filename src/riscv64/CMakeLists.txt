# src/riscv64/CMakeLists.txt — RISC-V 64-bit architecture sources

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/bootstrap.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/builtins.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_sbi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/counter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cpu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ec_arch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/entry.S
    ${CMAKE_CURRENT_SOURCE_DIR}/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interrupt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/plic.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptab_hpt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_hst.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/start.S
    ${CMAKE_CURRENT_SOURCE_DIR}/timer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utcb_arch.cpp
)
