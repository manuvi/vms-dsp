#include <cassert>
#include <cmath>

#include <vms/dsp/buffer.h>
#include <vms/dsp/simd.h>
#include <vms/dsp/simd_common.h>

namespace vms::dsp::detail
{
    const SimdBackend& backend();
}

namespace
{
    constexpr float scalar_factor = 0.5f;

    bool approx_equal(float a, float b) // helper tolerance to tolerate float noise
    {
        return std::fabs(a - b) < 1e-5f;
    }

    void fill_sequence(vms::dsp::Buffer<float>& buffer, float offset, float scale) // deterministic data generator
    {
        for (std::size_t i = 0; i < buffer.size(); ++i)
        {
            buffer[i] = offset + scale * static_cast<float>(i);
        }
    }

    void run_case(std::size_t size) // executes all SIMD ops for the requested length
    {
        using namespace vms::dsp;

        Buffer<float> a(size, BufferAlignment::Align32);
        Buffer<float> b(size, BufferAlignment::Align32);
        Buffer<float> c(size, BufferAlignment::Align32);
        Buffer<float> result(size, BufferAlignment::Align32);

        fill_sequence(a, 1.0f, 1.0f);
        fill_sequence(b, 0.5f, 0.25f);
        fill_sequence(c, -2.0f, 0.75f);

        const auto alignment = static_cast<std::size_t>(BufferAlignment::Align32); // request aligned SIMD paths

        if (size == 0)
        {
            return;
        }

        vector_add(a.data(), b.data(), result.data(), size, alignment); // add should use SIMD for aligned chunk
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(approx_equal(result[i], a[i] + b[i]));
        }

        vector_multiply(a.data(), b.data(), result.data(), size, alignment); // verify multiplication with scalar remainder for tail
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(approx_equal(result[i], a[i] * b[i]));
        }

        vector_multiply_scalar(a.data(), scalar_factor, result.data(), size, alignment); // multiply by scalar constant
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(approx_equal(result[i], a[i] * scalar_factor));
        }

        vector_multiply_add(a.data(), b.data(), c.data(), result.data(), size, alignment); // fused multiply-add path
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(approx_equal(result[i], (a[i] * b[i]) + c[i]));
        }

        vector_multiply_scalar_add(a.data(), scalar_factor, c.data(), result.data(), size, alignment); // scalar multiply + add
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(approx_equal(result[i], (a[i] * scalar_factor) + c[i]));
        }
    }
}

int main()
{
    using namespace vms::dsp::detail;

    const auto& simd = backend();
    const std::size_t block = simd.add.block > 0 ? static_cast<std::size_t>(simd.add.block) : 1; // discover runtime SIMD block

    const std::size_t below_block = block > 1 ? block - 1 : 1; // forces pure scalar path
    const std::size_t equal_block = block; // exactly one SIMD block
    const std::size_t above_block = block + (block > 1 ? (block / 2 > 0 ? block / 2 : 1) : 1); // includes tail remainder

    run_case(below_block);
    run_case(equal_block);
    run_case(above_block);

    return 0;
}
