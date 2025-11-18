#include <cassert>
#include <numeric>

#include <vms/dsp/buffer.h>
#include <vms/dsp/dsp.h>

int main()
{
    using namespace vms::dsp;

    Buffer<float> buffer(16, BufferAlignment::Align32); // allocate aligned storage
    assert(buffer.size() == 16);
    assert(buffer.is_aligned());

    auto view = buffer.view();
    std::iota(view.begin(), view.end(), 0.0f); // populate predictable pattern
    for (std::size_t i = 0; i < view.size(); ++i)
    {
        assert(view[i] == static_cast<float>(i));
    }

    Buffer<float> copy; // default-constructed buffer to test assign_from
    copy.assign_from(view);
    auto copy_view = copy.view();
    for (std::size_t i = 0; i < copy_view.size(); ++i)
    {
        assert(copy_view[i] == static_cast<float>(i));
    }

    Buffer<float> sum_result(buffer.size(), BufferAlignment::Align32); // dedicated buffer for SIMD result
    auto sum_view = sum_result.view();
    add(BufferView<const float>{buffer.view()}, BufferView<const float>{copy.view()}, sum_view); // SIMD add on aligned views
    for (std::size_t i = 0; i < sum_view.size(); ++i) // doubled pattern expected
    {
        assert(sum_view[i] == static_cast<float>(i * 2));
    }

    return 0;
}
