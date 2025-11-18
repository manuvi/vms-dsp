cmake_minimum_required(VERSION 3.20)

project(vms-dsp
    VERSION 0.1.0
    DESCRIPTION "VMS DSP Utilities Library"
    LANGUAGES C CXX
)

# Configurazione generale
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(SimdBackends)

set(VMS_DSP_SOURCES
    src/simd.cpp
    src/simd_default.cpp
)

vms_dsp_collect_backend_sources(VMS_DSP_SOURCES)

add_library(vms-dsp ${VMS_DSP_SOURCES})

target_include_directories(vms-dsp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vms-dsp
    PUBLIC
        vms-core
)


include(FetchContent)

# Bring in the vms-core dependency.
FetchContent_Declare(vms_core
    GIT_REPOSITORY https://github.com/manuvi/vms-core.git
    GIT_TAG main
)
FetchContent_MakeAvailable(vms_core)

enable_testing()
add_subdirectory(tests)