#pragma once

#include <cstddef>
#include <cstdint>

namespace vms::dsp::detail::scalar
{
    void fmuladd_scalar(const float* MulA, const float*  MulB, const float* AddC, float* Res, const std::size_t len);
    void fmulcadd_scalar(const float* Mul, const float*  Const, const float* AddC, float* Res, const std::size_t len);
    void fadd_scalar(const float* AddA, const float*  AddB, float* Res, const std::size_t len);
    void fmulc_scalar(const float* Mul, const float*  Const, float* Res, const std::size_t len);
    void fmul_scalar(const float* MulA, const float* MulB, float* Res, const std::size_t len);
} // namespace vms::dsp::detail::scalar
