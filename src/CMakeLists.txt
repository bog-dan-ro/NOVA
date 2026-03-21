# src/CMakeLists.txt — Common sources and architecture/feature dispatch

# ---------------------------------------------------------------------------
# Common sources (shared by all architectures)
# ---------------------------------------------------------------------------

target_sources(nova PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/buddy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/cmdline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console_mbuf.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/head_mbi.S
    ${CMAKE_CURRENT_SOURCE_DIR}/hip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pci.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pd.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pt.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ptab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rcu.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scheduler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/slab.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_mem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/space_obj.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/syscall.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout_budget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/timeout_hypercall.cpp
)

# integrity.cpp has an arch override for x86_32
if(NOT NOVA_ARCH STREQUAL "x86_32")
    target_sources(nova PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/integrity.cpp)
endif()

# ---------------------------------------------------------------------------
# Architecture-specific sources
# ---------------------------------------------------------------------------

add_subdirectory(${NOVA_ARCH})

# ---------------------------------------------------------------------------
# Feature-specific sources
# ---------------------------------------------------------------------------

foreach(_feature IN LISTS NOVA_FEATURES)
    set(_generic "${CMAKE_CURRENT_SOURCE_DIR}/generic/${_feature}")
    if(IS_DIRECTORY "${_generic}")
        add_subdirectory("generic/${_feature}")
    endif()

    set(_arch "${CMAKE_CURRENT_SOURCE_DIR}/${NOVA_ARCH}/${_feature}")
    if(IS_DIRECTORY "${_arch}")
        add_subdirectory("${NOVA_ARCH}/${_feature}")
    endif()
endforeach()
