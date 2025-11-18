#pragma once

#include <cstddef>
#include <cstdint>

namespace vms::dsp::detail::x86
{
    void fmuladd_fma(const float* MulA, const float*  MulB, const float* AddC, float* Res, std::size_t len, bool aligned);
    void fmulcadd_fma(const float* Mul, const float*  Const, const float* AddC, float* Res, std::size_t len, bool aligned);

} // namespace vms::dsp::detail::x86
