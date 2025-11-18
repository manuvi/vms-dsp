#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <vector>

#include <vms/dsp/buffer.h>
#include <vms/dsp/dsp.h>

int main()
{
    using namespace vms::dsp;

    constexpr std::size_t count = 32;

    // std::vector storage is not guaranteed to be 32-byte aligned; ensure we capture the failure path.
    {
        std::vector<float> a(count, 1.0f);
        std::vector<float> b(count, 2.0f);
        std::vector<float> out(count, 0.0f);

        BufferView<const float> view_a(a.data(), a.size(), BufferAlignment::Align32);
        BufferView<const float> view_b(b.data(), b.size(), BufferAlignment::Align32);
        BufferView<float> view_out(out.data(), out.size(), BufferAlignment::Align32);

        bool misalignment_handled = false;
        try // simulate the runtime check performed by SIMD helpers
        {
            if (!view_a.is_aligned() || !view_b.is_aligned() || !view_out.is_aligned())
            {
                throw std::runtime_error("std::vector buffers are not 32-byte aligned");
            }

            add(view_a, view_b, view_out);
        }
        catch (const std::runtime_error&)
        {
            misalignment_handled = true;
        }

        assert(misalignment_handled);
    }

    // Exercise the SIMD wrappers with properly aligned storage allocated via Buffer.
    {
        Buffer<float> buffer_a(count, BufferAlignment::Align32);
        Buffer<float> buffer_b(count, BufferAlignment::Align32);
        Buffer<float> buffer_out(count, BufferAlignment::Align32);

        std::fill_n(buffer_a.data(), buffer_a.size(), 1.0f); // fill predictable data for deterministic results
        std::fill_n(buffer_b.data(), buffer_b.size(), 2.0f);
        buffer_out.clear();

        const auto view_a = BufferView<const float>{buffer_a.view()};
        const auto view_b = BufferView<const float>{buffer_b.view()};
        auto view_out = buffer_out.view();

        assert(view_a.is_aligned());
        assert(view_b.is_aligned());
        assert(view_out.is_aligned());

        add(view_a, view_b, view_out); // SIMD path should run thanks to alignment
        for (std::size_t i = 0; i < view_out.size(); ++i)
        {
            assert(view_out[i] == 3.0f);
        }

        multiply(view_a, view_b, view_out); // multiply should also yield deterministic results
        for (std::size_t i = 0; i < view_out.size(); ++i)
        {
            assert(view_out[i] == 2.0f);
        }
    }

    return 0;
}
