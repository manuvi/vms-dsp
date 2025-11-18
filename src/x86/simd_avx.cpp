/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#include <vms/dsp/x86/simd_avx.h>

#include <immintrin.h>

namespace vms::dsp::detail::x86
{
namespace
{
    inline __m256 load_ps(const float* ptr, bool aligned)
    {
        return aligned ? _mm256_load_ps(ptr) : _mm256_loadu_ps(ptr);
    }

    inline void store_ps(float* ptr, __m256 value, bool aligned)
    {
        if (aligned)
        {
            _mm256_store_ps(ptr, value);
        }
        else
        {
            _mm256_storeu_ps(ptr, value);
        }
    }
}

void fmuladd_avx(const float* MulA, const float*  MulB, const float* AddC, float* Res, std::size_t len, bool aligned)
{
    for ( std::size_t i = 0 ; i < len ; i+= 8)
    {
        const __m256 mm_mula = load_ps(MulA + i, aligned);
        const __m256 mm_mulb = load_ps(MulB + i, aligned);
        const __m256 mm_add = load_ps(AddC + i, aligned);
        __m256 mm_res = _mm256_mul_ps(mm_mula, mm_mulb);
        mm_res = _mm256_add_ps(mm_res, mm_add);
        store_ps(Res + i, mm_res, aligned);
    }
}

void fmulcadd_avx(const float* Mul, const float*  Const, const float* AddC, float* Res, std::size_t len, bool aligned)
{
    const __m256 mm_const = _mm256_set1_ps(*Const);
    for ( std::size_t i = 0 ; i < len ; i+= 8)
    {
        const __m256 mm_mula = load_ps(Mul + i, aligned);
        const __m256 mm_add = load_ps(AddC + i, aligned);
        __m256 mm_res = _mm256_mul_ps(mm_mula, mm_const);
        mm_res = _mm256_add_ps(mm_res, mm_add);
        store_ps(Res + i, mm_res, aligned);
    }
}

void fadd_avx(const float* AddA, const float*  AddB, float* Res, std::size_t len, bool aligned)
{
    for ( std::size_t i = 0 ; i < len ; i+= 8)
    {
        const __m256 mm_suma = load_ps(AddA + i, aligned);
        const __m256 mm_sumb = load_ps(AddB + i, aligned);
        const __m256 mm_res = _mm256_add_ps(mm_suma, mm_sumb);
        store_ps(Res + i, mm_res, aligned);
    }
}

void fmulc_avx(const float* Mul, const float*  Const, float* Res, std::size_t len, bool aligned)
{
    const __m256 mm_op1 = _mm256_set1_ps(*Const);
    for ( std::size_t i = 0 ; i < len ; i+= 8)
    {
        const __m256 mm_op2 = load_ps(Mul + i, aligned);
        const __m256 mm_res = _mm256_mul_ps(mm_op1, mm_op2);
        store_ps(Res + i, mm_res, aligned);
    }
}

void fmul_avx(const float* MulA, const float* MulB, float* Res, std::size_t len, bool aligned)
{
    for ( std::size_t i = 0 ; i < len ; i+= 8)
    {
        const __m256 mm_op1 = load_ps(MulA + i, aligned);
        const __m256 mm_op2 = load_ps(MulB + i, aligned);
        const __m256 mm_res = _mm256_mul_ps(mm_op1, mm_op2);
        store_ps(Res + i, mm_res, aligned);
    }
}

} // namespace vms::dsp::detail::x86
