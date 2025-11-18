/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <vms/dsp/buffer_view.h>
#include <vms/dsp/simd.h>

namespace vms::dsp
{
    namespace detail
    {
        template <typename T>
        inline void scalar_add(const T* a, const T* b, T* out, std::size_t size)
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                out[i] = a[i] + b[i];
            }
        }

        template <typename T>
        inline void scalar_mul(const T* a, const T* b, T* out, std::size_t size)
        {
            for (std::size_t i = 0; i < size; ++i)
            {
                out[i] = a[i] * b[i];
            }
        }

        template <typename T>
        inline void dispatch_add(BufferView<const T> a, BufferView<const T> b, BufferView<T> out)
        {
            if (a.size() != b.size() || b.size() != out.size())
            {
                return;
            }

            if constexpr (std::is_same_v<T, float>)
            {
                const std::size_t alignment = std::min({a.alignment_bytes(), b.alignment_bytes(), out.alignment_bytes()});
                vector_add(a.data(), b.data(), out.data(), a.size(), alignment);
            }
            else
            {
                scalar_add(a.data(), b.data(), out.data(), out.size());
            }
        }

        template <typename T>
        inline void dispatch_mul(BufferView<const T> a, BufferView<const T> b, BufferView<T> out)
        {
            if (a.size() != b.size() || b.size() != out.size())
            {
                return;
            }

            if constexpr (std::is_same_v<T, float>)
            {
                const std::size_t alignment = std::min({a.alignment_bytes(), b.alignment_bytes(), out.alignment_bytes()});
                vector_multiply(a.data(), b.data(), out.data(), a.size(), alignment);
            }
            else
            {
                scalar_mul(a.data(), b.data(), out.data(), out.size());
            }
        }
    } // namespace detail

    template <typename T>
    inline void add(BufferView<const T> a, BufferView<const T> b, BufferView<T> out)
    {
        detail::dispatch_add(a, b, out);
    }

    template <typename T>
    inline void multiply(BufferView<const T> a, BufferView<const T> b, BufferView<T> out)
    {
        detail::dispatch_mul(a, b, out);
    }

} // namespace vms::dsp
