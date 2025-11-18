#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <utility>

#include <vms/dsp/buffer_view.h>

namespace vms::dsp
{
    namespace detail
    {
        /**
         * @brief Custom deleter that releases aligned allocations.
         */
        template <typename T>
        struct AlignedDeleter
        {
            std::size_t alignment = alignof(T);

            void operator()(T* ptr) const noexcept
            {
                if (ptr)
                {
                    ::operator delete[](ptr, std::align_val_t{alignment});
                }
            }
        };
    } // namespace detail

    /**
     * @brief Owning buffer that guarantees alignment and exposes `BufferView` interfaces.
     *
     * @tparam T element type.
     */
    template <typename T>
    class Buffer
    {
    public:
        using value_type = T;
        using View = BufferView<T>;
        using ConstView = BufferView<const T>;

        /**
         * @brief Constructs an empty buffer with the requested alignment.
         */
        explicit Buffer(BufferAlignment alignment = BufferAlignment::Align32)
            : alignment_(alignment)
            , alignment_bytes_(detail::normalize_alignment<T>(alignment))
            , storage_(nullptr, detail::AlignedDeleter<T>{alignment_bytes_})
        {
        }

        /**
         * @brief Constructs and allocates storage for @p size elements.
         */
        Buffer(std::size_t size, BufferAlignment alignment = BufferAlignment::Align32)
            : Buffer(alignment)
        {
            resize(size);
        }

        Buffer(const Buffer&) = delete;
        Buffer& operator=(const Buffer&) = delete;
        Buffer(Buffer&&) noexcept = default;
        Buffer& operator=(Buffer&&) noexcept = default;
        ~Buffer() = default;

        /// @name Introspection
        /// @{
        [[nodiscard]] std::size_t size() const noexcept { return size_; }
        [[nodiscard]] std::size_t memsize() const noexcept { return size_ * sizeof(T); }
        [[nodiscard]] BufferAlignment alignment() const noexcept { return alignment_; }
        [[nodiscard]] std::size_t alignment_bytes() const noexcept { return alignment_bytes_; }
        [[nodiscard]] bool is_aligned() const noexcept { return detail::is_pointer_aligned(storage_.get(), alignment_bytes_); }
        /// @}

        [[nodiscard]] T* data() noexcept { return storage_.get(); }
        [[nodiscard]] const T* data() const noexcept { return storage_.get(); }

        [[nodiscard]] View view() noexcept { return View{data(), size_, alignment_bytes_}; }
        [[nodiscard]] ConstView view() const noexcept { return ConstView{data(), size_, alignment_bytes_}; }

        T& operator[](std::size_t index) noexcept { return data()[index]; }
        const T& operator[](std::size_t index) const noexcept { return data()[index]; }

        /**
         * @brief Fills the buffer with zeros (no-op if empty).
         */
        void clear() noexcept
        {
            if (storage_ && size_ > 0)
            {
                std::memset(storage_.get(), 0, memsize());
            }
        }

        /**
         * @brief Resizes the buffer preserving the common prefix of existing data.
         */
        void resize(std::size_t size)
        {
            if (size == size_)
            {
                return;
            }

            if (size == 0)
            {
                storage_.reset();
                size_ = 0;
                return;
            }

            auto new_storage = allocate(size);
            if (storage_)
            {
                std::copy_n(storage_.get(), std::min(size, size_), new_storage.get());
            }

            storage_ = std::move(new_storage);
            size_ = size;
        }

        /**
         * @brief Resizes and copies the entirety of @p source into this buffer.
         */
        void assign_from(ConstView source)
        {
            resize(source.size());
            if (!source.empty())
            {
                std::copy(source.begin(), source.end(), data());
            }
        }

        /**
         * @brief Copies all data from @p source, resizing when needed.
         *
         * @return number of elements copied.
         */
        std::size_t copy_from(ConstView source)
        {
            resize(source.size());
            return dsp::copy(source, view());
        }

        /**
         * @brief Copies data into @p destination without resizing it.
         *
         * @return number of elements copied.
         */
        std::size_t copy_to(View destination) const
        {
            return dsp::copy(view(), destination);
        }

    private:
        std::unique_ptr<T, detail::AlignedDeleter<T>> allocate(std::size_t size) const
        {
            std::unique_ptr<T, detail::AlignedDeleter<T>> block{nullptr, detail::AlignedDeleter<T>{alignment_bytes_}};
            T* ptr = static_cast<T*>(::operator new[](size * sizeof(T), std::align_val_t{alignment_bytes_}));
            block.reset(ptr);
            block.get_deleter().alignment = alignment_bytes_;
            return block;
        }

        BufferAlignment alignment_;
        std::size_t alignment_bytes_;
        std::unique_ptr<T, detail::AlignedDeleter<T>> storage_;
        std::size_t size_ = 0;
    };

} // namespace vms::dsp
