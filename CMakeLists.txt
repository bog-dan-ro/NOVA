#
# CMakeLists.txt
#
# Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
# Economic rights: Technische Universitaet Dresden (Germany)
#
# Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
# Copyright (C) 2019-2025 Udo Steinberg, BlueRock Security, Inc.
#
# This file is part of the NOVA microhypervisor.
#
# NOVA is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# NOVA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License version 2 for more details.
#

cmake_minimum_required(VERSION 3.31)

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

set(NOVA_ARCH  "x86_64" CACHE STRING "Target architecture")
set(NOVA_BOARD "pc"     CACHE STRING "Target board")
set(NOVA_CFP   "none"   CACHE STRING "Control-flow protection")

set_property(CACHE NOVA_ARCH PROPERTY STRINGS aarch64 riscv64 x86_32 x86_64)
set_property(CACHE NOVA_CFP  PROPERTY STRINGS none branch return full)

# ---------------------------------------------------------------------------
# Bare-metal setup
# When NOVA is the top-level project, configure cross-compilation.
# When used via FetchContent / add_subdirectory, the parent project
# must provide a suitable cross-compilation toolchain file.
# ---------------------------------------------------------------------------

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CMAKE_SYSTEM_NAME      Generic)
    set(CMAKE_SYSTEM_PROCESSOR ${NOVA_ARCH})
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
endif()

project(nova LANGUAGES CXX ASM)

# ---------------------------------------------------------------------------
# Validation
# ---------------------------------------------------------------------------

set(_valid_archs aarch64 riscv64 x86_32 x86_64)

if(NOT NOVA_ARCH IN_LIST _valid_archs)
    message(FATAL_ERROR "NOVA_ARCH '${NOVA_ARCH}' is invalid. Valid: ${_valid_archs}")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/conf/boards/${NOVA_ARCH}/${NOVA_BOARD}.cmake")
    message(FATAL_ERROR "Board '${NOVA_BOARD}' not found: conf/boards/${NOVA_ARCH}/${NOVA_BOARD}.cmake")
endif()

# ---------------------------------------------------------------------------
# Include architecture and board configuration
# ---------------------------------------------------------------------------

# Arch config sets: NOVA_ARCH_FLAGS, NOVA_ARCH_DEFINES, NOVA_ARCH_WFLAGS,
#                   NOVA_ARCH_LFLAGS, NOVA_OUTPUT_NAME
include(${CMAKE_CURRENT_SOURCE_DIR}/conf/archs/${NOVA_ARCH}.cmake)

# Board config sets: NOVA_FEATURES
# Board config may set: NOVA_LINKER_SCRIPT, and append to NOVA_BOARD_SOURCES
include(${CMAKE_CURRENT_SOURCE_DIR}/conf/boards/${NOVA_ARCH}/${NOVA_BOARD}.cmake)

# ---------------------------------------------------------------------------
# Linker script resolution (board > arch > fallback)
# ---------------------------------------------------------------------------

if(NOT DEFINED NOVA_LINKER_SCRIPT)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${NOVA_ARCH}/hypervisor.ld")
        set(NOVA_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/${NOVA_ARCH}/hypervisor.ld")
    else()
        set(NOVA_LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/src/hypervisor.ld")
    endif()
endif()

message(STATUS "NOVA arch      : ${NOVA_ARCH}")
message(STATUS "NOVA board     : ${NOVA_BOARD}")
message(STATUS "NOVA features  : ${NOVA_FEATURES}")
message(STATUS "NOVA linker ld : ${NOVA_LINKER_SCRIPT}")

# ---------------------------------------------------------------------------
# Git version
# ---------------------------------------------------------------------------

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE   _git_rev
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    RESULT_VARIABLE   _git_result
)

if(NOT _git_result EQUAL 0)
    set(_git_rev "0000000")
endif()

string(SUBSTRING "${_git_rev}" 0 7 NOVA_GIT_VER)

# ---------------------------------------------------------------------------
# Use ld directly for linking (matching Makefile behaviour)
# ---------------------------------------------------------------------------

set(CMAKE_CXX_LINK_EXECUTABLE
    "<CMAKE_LINKER> <LINK_FLAGS> <OBJECTS> -o <TARGET>")

# ---------------------------------------------------------------------------
# Hypervisor target
# ---------------------------------------------------------------------------

add_executable(nova)

# ---------------------------------------------------------------------------
# C++ standard — prefer C++26, fall back to C++23
# ---------------------------------------------------------------------------

include(CheckCXXCompilerFlag)

check_cxx_compiler_flag(-std=gnu++26 _nova_has_cxx26)

if(_nova_has_cxx26)
    set_target_properties(nova PROPERTIES CXX_STANDARD 26)
else()
    set_target_properties(nova PROPERTIES CXX_STANDARD 23)
endif()

set_target_properties(nova PROPERTIES
    CXX_EXTENSIONS        ON
    CXX_STANDARD_REQUIRED ON
)

# ---------------------------------------------------------------------------
# Common compile options
# ---------------------------------------------------------------------------

target_compile_options(nova PRIVATE
    -pipe
    -Os
    -ffreestanding
    -fdata-sections
    -ffunction-sections
    -fdiagnostics-color=auto
    -fno-asynchronous-unwind-tables
    -fno-exceptions
    -fno-pic
    -fno-rtti
    -fno-stack-protector
    -fno-use-cxa-atexit
    -fomit-frame-pointer
)

# Control-flow protection
check_cxx_compiler_flag("-fcf-protection=${NOVA_CFP}" _nova_has_cfp)
if(_nova_has_cfp)
    target_compile_options(nova PRIVATE "-fcf-protection=${NOVA_CFP}")
endif()

# ---------------------------------------------------------------------------
# Warning options
# ---------------------------------------------------------------------------

target_compile_options(nova PRIVATE
    -Wall -Wextra
    -Walloca -Wcast-align -Wcast-qual -Wconversion
    -Wctor-dtor-privacy -Wdisabled-optimization
    -Wduplicated-branches -Wduplicated-cond
    -Wenum-conversion -Wextra-semi -Wformat=2
    -Wlogical-op -Wmismatched-tags
    -Wmissing-format-attribute -Wmissing-noreturn -Wmultichar
    -Wnoexcept -Wold-style-cast -Woverloaded-virtual
    -Wpacked -Wpointer-arith
    -Wredundant-decls -Wredundant-tags -Wregister -Wshadow
    -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5
    -Wsuggest-override -Wvirtual-inheritance
    -Wvolatile -Wvolatile-register-var
    -Wwrite-strings -Wzero-as-null-pointer-constant
)

# Optional warnings (requires newer GCC versions)
foreach(_flag IN ITEMS
    -Wbidi-chars=any            # gcc-12
    -Wnrvo                     # gcc-14
    -Wleading-whitespace=spaces # gcc-15
    -Wtrailing-whitespace=any   # gcc-15
)
    string(MAKE_C_IDENTIFIER "nova_w_${_flag}" _var)
    check_cxx_compiler_flag("${_flag}" ${_var})
    if(${_var})
        target_compile_options(nova PRIVATE ${_flag})
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Architecture-specific compile options
# ---------------------------------------------------------------------------

target_compile_options(nova PRIVATE ${NOVA_ARCH_FLAGS})

# Architecture-specific warnings (checked individually)
foreach(_flag IN LISTS NOVA_ARCH_WFLAGS)
    string(MAKE_C_IDENTIFIER "nova_aw_${_flag}" _var)
    check_cxx_compiler_flag("${_flag}" ${_var})
    if(${_var})
        target_compile_options(nova PRIVATE ${_flag})
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Preprocessor defines
# ---------------------------------------------------------------------------

set(NOVA_DEFINES "")

foreach(_feature IN LISTS NOVA_FEATURES)
    list(APPEND NOVA_DEFINES FEATURE_${_feature})
endforeach()

if(DEFINED NOVA_ARCH_DEFINES)
    list(APPEND NOVA_DEFINES ${NOVA_ARCH_DEFINES})
endif()

target_compile_definitions(nova PRIVATE ${NOVA_DEFINES})

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------

set(NOVA_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/${NOVA_ARCH}
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

foreach(_feature IN LISTS NOVA_FEATURES)
    foreach(_dir IN ITEMS
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/${_feature}"
        "${CMAKE_CURRENT_SOURCE_DIR}/inc/${NOVA_ARCH}/${_feature}"
    )
        if(IS_DIRECTORY "${_dir}")
            list(APPEND NOVA_INCLUDE_DIRS "${_dir}")
        endif()
    endforeach()
endforeach()

target_include_directories(nova PRIVATE ${NOVA_INCLUDE_DIRS})

# ---------------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------------

add_subdirectory(src)
add_subdirectory(inc)

# Board-specific extra sources (set by board cmake files)
if(DEFINED NOVA_BOARD_SOURCES)
    target_sources(nova PRIVATE ${NOVA_BOARD_SOURCES})
endif()

# ---------------------------------------------------------------------------
# Preprocess linker script
# ---------------------------------------------------------------------------

set(NOVA_LD_PREPROCESSED "${CMAKE_CURRENT_BINARY_DIR}/hypervisor.ld")

set(_pp_flags -fno-pic ${NOVA_ARCH_FLAGS})
foreach(_dir IN LISTS NOVA_INCLUDE_DIRS)
    list(APPEND _pp_flags "-I${_dir}")
endforeach()
foreach(_def IN LISTS NOVA_DEFINES)
    list(APPEND _pp_flags "-D${_def}")
endforeach()

add_custom_command(
    OUTPUT  "${NOVA_LD_PREPROCESSED}"
    COMMAND ${CMAKE_CXX_COMPILER} ${_pp_flags}
            -xassembler-with-cpp -E -P
            -o "${NOVA_LD_PREPROCESSED}"
            "${NOVA_LINKER_SCRIPT}"
    DEPENDS "${NOVA_LINKER_SCRIPT}"
    COMMENT "Preprocessing linker script"
    VERBATIM
)

add_custom_target(nova_linker_script DEPENDS "${NOVA_LD_PREPROCESSED}")
add_dependencies(nova nova_linker_script)

# ---------------------------------------------------------------------------
# Linker options
# ---------------------------------------------------------------------------

target_link_options(nova PRIVATE
    "--defsym=GIT_VER=0x${NOVA_GIT_VER}"
    --gc-sections
    --warn-common
    -static -n -s
    -z muldefs
    -T "${NOVA_LD_PREPROCESSED}"
    ${NOVA_ARCH_LFLAGS}
)

set_target_properties(nova PROPERTIES
    LINK_DEPENDS "${NOVA_LD_PREPROCESSED}"
    OUTPUT_NAME  "${NOVA_OUTPUT_NAME}"
    SUFFIX       ""
)
