/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace vms::dsp
{
    /**
     * @brief Supported alignment sizes (in bytes) for DSP buffers.
     */
    enum class BufferAlignment : std::size_t
    {
        Align8  = 8,
        Align16 = 16,
        Align32 = 32,
        Align64 = 64
    };

    namespace detail
    {
        /**
         * @brief Returns the alignment value expressed in bytes.
         */
        constexpr std::size_t alignment_value(BufferAlignment alignment) noexcept
        {
            return static_cast<std::size_t>(alignment);
        }

        template <typename T>
        /**
         * @brief Ensures the requested alignment is at least the natural alignment of @p T.
         */
        constexpr std::size_t normalize_alignment(BufferAlignment alignment) noexcept
        {
            const auto requested = alignment_value(alignment);
            const auto required = alignof(T);
            return requested < required ? required : requested;
        }

        template <typename T>
        /**
         * @brief Ensures the runtime alignment value is compatible with @p T.
         */
        constexpr std::size_t normalize_alignment(std::size_t alignment_bytes) noexcept
        {
            const auto required = alignof(T);
            return alignment_bytes < required ? required : alignment_bytes;
        }

        template <typename T>
        /**
         * @brief Checks whether @p ptr satisfies the requested alignment.
         */
        constexpr bool is_pointer_aligned(const T* ptr, std::size_t alignment) noexcept
        {
            if (ptr == nullptr)
            {
                return true;
            }

            const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
            return (addr % alignment) == 0;
        }
    } // namespace detail

} // namespace vms::dsp
