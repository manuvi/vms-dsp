/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#pragma once

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <vms/dsp/buffer_alignment.h>

namespace vms::dsp
{
    /**
     * @brief Non-owning view over a contiguous block of memory with alignment metadata.
     *
     * @tparam T element type (can be const to express read-only access).
     */
    template <typename T>
    class BufferView
    {
    public:
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator = pointer;
        using const_iterator = const T*;

        /**
         * @brief Constructs an empty view.
         */
        constexpr BufferView() = default;

        /**
         * @brief Constructs a view from pointer, size, and enum-based alignment.
         */
        constexpr BufferView(pointer data, std::size_t size, BufferAlignment alignment) noexcept
            : BufferView(data, size, detail::alignment_value(alignment))
        {
        }

        /**
         * @brief Constructs a view from pointer, size, and raw alignment in bytes.
         */
        constexpr BufferView(pointer data, std::size_t size, std::size_t alignment_bytes = alignof(T)) noexcept
            : data_(data)
            , size_(size)
            , alignment_bytes_(detail::normalize_alignment<std::remove_const_t<T>>(alignment_bytes))
        {
        }

        /**
         * @brief Enables implicit conversion from mutable to const view.
         */
        template <typename U, typename = std::enable_if_t<std::is_const_v<T> && std::is_same_v<U, std::remove_const_t<T>>>>
        BufferView(const BufferView<U>& other) noexcept
            : data_(other.data())
            , size_(other.size())
            , alignment_bytes_(other.alignment_bytes())
        {
        }

        /// @name Introspection
        /// @{
        [[nodiscard]] pointer data() const noexcept { return data_; }
        [[nodiscard]] std::size_t size() const noexcept { return size_; }
        [[nodiscard]] std::size_t memsize() const noexcept { return size_ * sizeof(T); }
        [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
        [[nodiscard]] std::size_t alignment_bytes() const noexcept { return alignment_bytes_; }
        [[nodiscard]] bool is_aligned() const noexcept { return detail::is_pointer_aligned(data_, alignment_bytes_); }
        /// @}

        iterator begin() const noexcept { return data_; }
        iterator end() const noexcept { return data_ + size_; }

        reference operator[](std::size_t index) const noexcept { return data_[index]; }

        reference at(std::size_t index) const
        {
            if (index >= size_)
            {
                throw std::out_of_range("BufferView::at");
            }
            return data_[index];
        }

        /**
         * @brief Returns a view clamped to the requested window without copying data.
         */
        BufferView subview(std::size_t offset, std::size_t count) const noexcept
        {
            if (offset >= size_)
            {
                return {};
            }

            const auto clamped = std::min(count, size_ - offset);
            return BufferView{data_ + offset, clamped, alignment_bytes_};
        }

        /**
         * @brief Rebinds the view to new memory using enum-based alignment.
         */
        void reset(pointer data, std::size_t size, BufferAlignment alignment) noexcept
        {
            reset(data, size, detail::alignment_value(alignment));
        }

        /**
         * @brief Rebinds the view to new memory using raw alignment (bytes).
         */
        void reset(pointer data, std::size_t size, std::size_t alignment_bytes = alignof(T)) noexcept
        {
            data_ = data;
            size_ = size;
            alignment_bytes_ = detail::normalize_alignment<std::remove_const_t<T>>(alignment_bytes);
        }

    private:
        pointer data_ = nullptr;
        std::size_t size_ = 0;
        std::size_t alignment_bytes_ = alignof(T);
    };

    /**
     * @brief Convenience alias mirroring the legacy pointer-based terminology.
     */
    template <typename T>
    using BufferPtr = BufferView<T>;

    /**
     * @brief Copies as many samples as possible from @p src to @p dst.
     *
     * @return number of elements copied.
     */
    template <typename T>
    std::size_t copy(BufferView<const T> src, BufferView<T> dst)
    {
        const auto count = std::min(src.size(), dst.size());
        if (count == 0)
        {
            return 0;
        }

        std::copy_n(src.data(), count, dst.data());
        return count;
    }

} // namespace vms::dsp
