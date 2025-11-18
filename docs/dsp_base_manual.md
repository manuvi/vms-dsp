# vms::dsp `dsp_base` Manual

This document explains how to use the utilities defined in:

- `include/vms/dsp/buffer_alignment.h`
- `include/vms/dsp/buffer_view.h`
- `include/vms/dsp/buffer.h`
- convenience umbrella header `include/vms/dsp/dsp_base.h`

It also walks through the relevant C++ syntax so the code can be adapted or extended easily.

---

## 1. Overview

`dsp_base` (and the headers it includes) provides three kinds of tools:

1. **Alignment descriptors** (`BufferAlignment`, helper functions)  
   Describe the byte alignment you need (8/16/32/64) to satisfy SIMD or cache requirements.
2. **Non-owning buffer views** (`BufferView<T>` and its alias `BufferPtr<T>`)  
   Lightweight objects that wrap a raw pointer + size + alignment information.
3. **Owning buffers** (`Buffer<T>`)  
   Containers that allocate memory using the specified alignment and expose views.

The design is header-only and does not require RTTI or virtual inheritance. It relies heavily on C++ templates, constexpr functions, and RAII principles (`std::unique_ptr` with custom deleters) to remain fast and safe.

---

## 2. Alignment utilities

### `enum class BufferAlignment : std::size_t`

```cpp
enum class BufferAlignment : std::size_t {
    Align8  = 8,
    Align16 = 16,
    Align32 = 32,
    Align64 = 64
};
```

This strongly typed enum restricts alignment requests to the supported byte values.  
Because it is scoped (`enum class`) you must prefix the enumerators: `BufferAlignment::Align32`.

Internally we convert the enum to a numeric value via `static_cast<std::size_t>` inside `detail::alignment_in_bytes`.

### `detail::normalize_alignment<T>(BufferAlignment)`

```cpp
template <typename T>
constexpr std::size_t normalize_alignment(BufferAlignment alignment) noexcept;
```

`normalize_alignment` ensures we never ask the allocator for an alignment smaller than the natural alignment of `T`.  
It returns `max(requested_alignment, alignof(T))`.

This function is `constexpr`, meaning it can be evaluated at compile time when its arguments are constant expressions, reducing runtime cost.

### `detail::is_pointer_aligned<T>(const T*, std::size_t)`

Checks whether a pointer address is a multiple of the requested alignment.  
It casts the pointer to `std::uintptr_t` so we can perform modulo arithmetic on the address.

---

## 3. `BufferView<T>` (a.k.a `BufferPtr<T>`)

### Purpose

`BufferView<T>` is a non-owning object that bundles:

- `T* data_` — raw pointer to the first element  
- `std::size_t size_` — number of elements  
- `std::size_t alignment_bytes_` — guaranteed byte alignment

Because it does not allocate or free memory, you can copy it freely.  
`BufferPtr<T>` is simply `using BufferPtr = BufferView<T>;`.

### Constructors

```cpp
constexpr BufferView() = default;

constexpr BufferView(pointer data, std::size_t size,
                     BufferAlignment alignment = BufferAlignment::Align8) noexcept;

constexpr BufferView(pointer data, std::size_t size,
                     std::size_t alignment_bytes) noexcept;
```

The overload that takes `BufferAlignment` automatically normalizes the alignment.  
The second overload is useful if the alignment is known only at runtime.

### Const-view conversion

```cpp
template <typename U,
          typename = std::enable_if_t<std::is_const_v<T> && std::is_same_v<U, std::remove_const_t<T>>>>
BufferView(const BufferView<U>& other) noexcept;
```

This template constructor allows implicit conversion from `BufferView<float>` to `BufferView<const float>`.  
It uses SFINAE (`std::enable_if_t`) to make the constructor participate only when `T` is const and `U` is the matching non-const type.

### Member functions

- `data()/size()/memsize()/empty()` — standard container-style accessors
- `alignment_bytes()/is_aligned()` — introspection helpers
- `begin()/end()` — raw pointers so the view works with algorithms like `std::copy`
- `operator[]/at()` — element access (`at` throws `std::out_of_range` if out of bounds)
- `subview(offset, count)` — returns a view within the current view without copying
- `reset(ptr, size, alignment)` — rebinds the view to new memory

### Typical usage

```cpp
void process(BufferView<float> samples) {
    if (!samples.is_aligned()) {
        // fall back to scalar processing
    }
    for (float& x : samples) {
        x *= 0.5f;
    }
}

float* raw = get_audio_buffer();
BufferView<float> view{raw, num_samples, BufferAlignment::Align32};
process(view);
```

Because the type is trivially copyable, passing it by value is cheap.

---

## 4. `Buffer<T>` — owning, aligned storage

### Constructor summary

```cpp
explicit Buffer(BufferAlignment alignment = BufferAlignment::Align32);
Buffer(std::size_t size, BufferAlignment alignment = BufferAlignment::Align32);
```

The default constructor does not allocate until you call `resize`.  
Passing the size eagerly allocates aligned storage.

### RAII storage

The class holds a `std::unique_ptr<T, detail::AlignedDeleter<T>> data_;`.  
`AlignedDeleter` calls the sized `::operator delete[]` overload that takes a `std::align_val_t`, guaranteeing we release memory with the same alignment used for allocation.  
Because the deleter stores the alignment value, moving the buffer between objects is safe.

### Core methods

- `size()/memsize()` — number of elements / bytes  
- `alignment()/alignment_bytes()` — the requested enum and its numeric counterpart  
- `is_aligned()` — uses `detail::is_pointer_aligned` on the owned pointer  
- `data()` — raw pointer (non-const/const overloads)  
- `view()/const view()` — construct a `BufferView` referencing the owned memory  
- `operator[]` — unchecked element access  
- `clear()` — `std::memset` the buffer to zero  
- `resize(new_size)` — reallocate aligned memory; copies minimal portion of old data  
- `assign_from(BufferView<const T>)` — convenience for copying from any view

### Example: owning + non-owning interaction

```cpp
using namespace vms::dsp;

Buffer<float> temp{512, BufferAlignment::Align32};

// Fill buffer
std::iota(temp.data(), temp.data() + temp.size(), 0.0f);

// Pass to DSP code
auto process_block = [](BufferView<float> block) {
    for (float& sample : block) {
        sample = std::sin(sample);
    }
};

process_block(temp.view());
```

Because `BufferView` just wraps a pointer, functions can accept either a `Buffer` or any other memory source (stack arrays, mapped hardware buffers, etc.) as long as you can describe them with a view.

### Custom alignment

```cpp
Buffer<double> filter_taps(BufferAlignment::Align64);
filter_taps.resize(1024);
```

All addresses returned by `data()` and `view()` will be 64-byte aligned (or tighter, based on `alignof(double)`).

---

## 5. C++ language features used

1. **Templates**  
   `BufferView<T>` and `Buffer<T>` are class templates. You instantiate them by supplying a concrete type, e.g. `Buffer<float>`.

2. **`constexpr`**  
   Functions like `normalize_alignment` and constructors are marked `constexpr`, enabling compile-time evaluation when possible.

3. **`noexcept`**  
   Many functions guarantee they do not throw. This communicates intent and can improve optimizer decisions.

4. **`std::enable_if_t` / SFINAE**  
   Used to restrict template constructors to specific type combinations (const-to-non-const conversions).

5. **`alignas` / `std::align_val_t`**  
   The code uses `::operator new[](…, std::align_val_t{alignment})` and the matching delete to request specific alignments, a C++17 feature.

6. **`std::unique_ptr` with custom deleter**  
   Allows RAII management of aligned allocations without writing manual constructors/destructors.

7. **Move semantics**  
   `Buffer` deletes copy operations but allows moves (`Buffer(Buffer&&) noexcept = default`). Moving transfers ownership of the pointer and deleter.

8. **`std::min`, `std::copy_n`, `std::memset`**  
   Standard algorithms and C library calls simplify element management.

9. **`enum class`**  
   Provides scoped, type-safe enumerations.

10. **`std::uintptr_t`**  
    A standard unsigned integer type guaranteed to hold a pointer; used for alignment checks.

---

## 6. Common patterns

### Creating a view from an existing `Buffer`

```cpp
Buffer<float> storage(256);
BufferView<float> window = storage.view().subview(0, 128);
```

### Wrapping raw hardware buffers

```cpp
float* hardware_buffer = map_hardware();
BufferView<float> audio{hardware_buffer, frame_size, BufferAlignment::Align32};
```

### Copying between buffers

```cpp
Buffer<float> a(128), b(128);
a.assign_from(b.view());     // copies 128 elements

Buffer<float> c(64);
c.assign_from(a.view());     // resizes c to 128 and copies
```

### Passing views around

```cpp
void add(BufferView<const float> in_a,
         BufferView<const float> in_b,
         BufferView<float> out) {
    assert(in_a.size() == in_b.size());
    assert(out.size() == in_a.size());
    for (std::size_t i = 0; i < out.size(); ++i) {
        out[i] = in_a[i] + in_b[i];
    }
}
```

Because the view carries alignment metadata you can decide inside the function whether to call SIMD kernels or scalar fallbacks.

---

## 7. Error handling and safety

- Access via `operator[]` is unchecked, mirroring `std::span`. Use `at()` when you need bounds checking.
- `BufferView::subview` clamps the count to avoid running past the original buffer.
- `Buffer::resize` preserves as much data as possible when growing/shrinking but does not initialize new entries; call `clear()` or `std::fill` if you need zeros.
- `Buffer::assign_from` always resizes the destination, so be aware of potential reallocations in hot paths.

---

## 8. Extending the system

- To support custom allocators, wrap them inside `Buffer` or add a constructor that accepts a callable returning aligned memory.
- For thread-safe reference counting, replace `std::unique_ptr` with `std::shared_ptr` plus an aligned deleter; the surrounding API remains identical.
- `BufferView` can be trivially embedded into your own structs to describe interleaved channels, e.g. store two views and treat them as a stereo pair.

---

With these tools you can describe DSP data flows in a self-documenting way: owning buffers guarantee alignment and lifetimes, while views make function interfaces precise without forcing a specific allocation strategy. Use the alignment metadata to dispatch to the SIMD backend introduced in `simd.*` and keep scalar fallbacks for any misaligned cases. Happy hacking!
