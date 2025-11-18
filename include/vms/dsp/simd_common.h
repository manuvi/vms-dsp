/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace vms::dsp::detail
{
    template <typename Fn>
    struct SimdOp
    {
        Fn fn = nullptr;
        int32_t block = 1;
        std::size_t alignment_bytes = 1;

        constexpr explicit operator bool() const noexcept { return fn != nullptr; }
    };

    using vector_add_fn = void (*)(const float* AddA, const float*  AddB, float* Res, const std::size_t len, bool aligned);
    using vector_mul_fn = void (*)(const float* MulA, const float* MulB, float* Res, const std::size_t len, bool aligned);
    using vector_mul_scalar_fn = void (*)(const float* Mul, const float*  Const, float* Res, const std::size_t len, bool aligned);
    using vector_mul_add_fn = void (*)(const float* MulA, const float*  MulB, const float* AddC, float* Res, const std::size_t len, bool aligned);
    using vector_mul_scalar_add_fn = void (*)(const float* Mul, const float*  Const, const float* AddC, float* Res, const std::size_t len, bool aligned);

    struct SimdBackend
    {
        SimdOp<vector_add_fn> add;
        SimdOp<vector_mul_fn> multiply;
        SimdOp<vector_mul_scalar_fn> multiply_scalar;
        SimdOp<vector_mul_add_fn> multiply_add;
        SimdOp<vector_mul_scalar_add_fn> multiply_scalar_add;
    };

} // namespace vms::dsp::detail
