#pragma once

#include <cstddef>
#include <cstdint>

namespace vms::dsp::detail::x86
{
    void fmuladd_avx(const float* MulA, const float*  MulB, const float* AddC, float* Res, std::size_t len, bool aligned);
    void fmulcadd_avx(const float* Mul, const float*  Const, const float* AddC, float* Res, std::size_t len, bool aligned);
    void fadd_avx(const float* AddA, const float*  AddB, float* Res, std::size_t len, bool aligned);
    void fmulc_avx(const float* Mul, const float*  Const, float* Res, std::size_t len, bool aligned);
    void fmul_avx(const float* MulA, const float* MulB, float* Res, std::size_t len, bool aligned);
} // namespace vms::dsp::detail::x86
