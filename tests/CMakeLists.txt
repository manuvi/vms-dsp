cmake_minimum_required(VERSION 3.20)

include(CTest)

if(NOT BUILD_TESTING)
    return()
endif()

set(TEST_SOURCES
    test_buffers.cpp
    test_simd_block_fallback.cpp
    test_simd_wrappers.cpp
)

foreach(TEST_SOURCE IN LISTS TEST_SOURCES)
    get_filename_component(TEST_NAME "${TEST_SOURCE}" NAME_WE)
    add_executable(${TEST_NAME} "${TEST_SOURCE}")
    target_link_libraries(${TEST_NAME}
        PRIVATE
            vms-dsp
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
