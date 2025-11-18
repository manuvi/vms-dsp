/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace vms::dsp
{
    // Float SIMD entry points
    void vector_add(const float* InA, const float* InB, float* Out, std::size_t Sz, std::size_t alignment_bytes = 1);
    void vector_multiply(const float* InA, const float* InB, float* Out, std::size_t Sz, std::size_t alignment_bytes = 1);
    void vector_multiply_scalar(const float* In, const float& k, float* Out, std::size_t Sz, std::size_t alignment_bytes = 1);
    void vector_multiply_add(const float* InMulA, const float* InMulB, const float* InAddC, float* Out, std::size_t Sz, std::size_t alignment_bytes = 1);
    void vector_multiply_scalar_add(const float* InMul, const float& k, const float* InAdd, float* Out, std::size_t Sz, std::size_t alignment_bytes = 1);
}
