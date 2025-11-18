include_guard(GLOBAL)

include(CheckCXXCompilerFlag)

option(VMS_DSP_ENABLE_X86_AVX "Enable AVX backend (requires x86 + -mavx)" ON)
option(VMS_DSP_ENABLE_X86_FMA "Enable FMA backend (requires AVX + -mfma)" ON)

function(vms_dsp_collect_backend_sources out_var)
    set(sources ${${out_var}})

    set(VMS_DSP_ARCH_X86 OFF)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86_64|AMD64|i[3-6]86)")
        set(VMS_DSP_ARCH_X86 ON)
    endif()

    if(VMS_DSP_ENABLE_X86_AVX AND VMS_DSP_ARCH_X86)
        check_cxx_compiler_flag("-mavx" VMS_DSP_COMPILER_HAS_AVX)
        if(VMS_DSP_COMPILER_HAS_AVX)
            list(APPEND sources src/x86/simd_avx.cpp)
            set_source_files_properties(
                src/x86/simd_avx.cpp
                PROPERTIES
                    COMPILE_OPTIONS "-mavx"
            )
        else()
            message(WARNING "AVX backend requested but the compiler does not support -mavx")
        endif()
    elseif(VMS_DSP_ENABLE_X86_AVX)
        message(WARNING "AVX backend requested but the current architecture is not x86/AMD64; skipping")
    endif()

    if(VMS_DSP_ENABLE_X86_FMA AND VMS_DSP_ARCH_X86)
        check_cxx_compiler_flag("-mfma" VMS_DSP_COMPILER_HAS_FMA)
        if(VMS_DSP_COMPILER_HAS_AVX AND VMS_DSP_COMPILER_HAS_FMA)
            list(APPEND sources src/x86/simd_fma.cpp)
            set_source_files_properties(
                src/x86/simd_fma.cpp
                PROPERTIES
                    COMPILE_OPTIONS "-mavx;-mfma"
            )
        else()
            message(WARNING "FMA backend requested but the compiler/flags are not available (needs -mavx and -mfma)")
        endif()
    elseif(VMS_DSP_ENABLE_X86_FMA)
        message(WARNING "FMA backend requested but the current architecture is not x86/AMD64; skipping")
    endif()

    set(${out_var} "${sources}" PARENT_SCOPE)
endfunction()
