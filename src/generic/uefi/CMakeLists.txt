# src/generic/uefi/CMakeLists.txt — UEFI feature sources (shared)

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/console_fbuf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uefi.cpp
)
