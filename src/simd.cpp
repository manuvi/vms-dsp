#include <vms/dsp/simd.h>
#include <vms/core/singleton.h>
#include <vms/dsp/simd_common.h>
#include <vms/dsp/simd_default.h>

#if defined(__x86_64__)
#include <vms/dsp/x86/simd_avx.h>
#include <vms/dsp/x86/simd_fma.h>
#endif

namespace vms::dsp
{

namespace detail
{
    class SimdRuntime : public vms::core::Singleton<SimdRuntime>
    {
    public:
        SimdRuntime();

        const SimdBackend& backend() const { return backend_; }

    private:
        SimdBackend backend_;
    };

    namespace
    {
        template <typename Fn>
        SimdOp<Fn> make_op(Fn fn, int32_t block = 1, std::size_t alignment = 1)
        {
            SimdOp<Fn> op{};
            op.fn = fn;
            op.block = block;
            op.alignment_bytes = alignment > 0 ? alignment : 1;
            return op;
        }

        void scalar_add_adapter(const float* a, const float* b, float* out, std::size_t len, bool)
        {
            scalar::fadd_scalar(a, b, out, len);
        }

        void scalar_multiply_adapter(const float* a, const float* b, float* out, std::size_t len, bool)
        {
            scalar::fmul_scalar(a, b, out, len);
        }

        void scalar_multiply_scalar_adapter(const float* in, const float* k, float* out, std::size_t len, bool)
        {
            scalar::fmulc_scalar(in, k, out, len);
        }

        void scalar_multiply_add_adapter(const float* a, const float* b, const float* c, float* out, std::size_t len, bool)
        {
            scalar::fmuladd_scalar(a, b, c, out, len);
        }

        void scalar_multiply_scalar_add_adapter(const float* a, const float* k, const float* c, float* out, std::size_t len, bool)
        {
            scalar::fmulcadd_scalar(a, k, c, out, len);
        }

        SimdBackend make_scalar_backend()
        {
            SimdBackend backend{};
            backend.add = make_op(scalar_add_adapter);
            backend.multiply = make_op(scalar_multiply_adapter);
            backend.multiply_scalar = make_op(scalar_multiply_scalar_adapter);
            backend.multiply_add = make_op(scalar_multiply_add_adapter);
            backend.multiply_scalar_add = make_op(scalar_multiply_scalar_add_adapter);
            return backend;
        }

        template <typename Op>
        void adopt_op(Op& dst, const Op& src)
        {
            if (src.fn != nullptr)
            {
                dst.fn = src.fn;
                dst.block = src.block > 0 ? src.block : dst.block;
                if (src.alignment_bytes > dst.alignment_bytes)
                {
                    dst.alignment_bytes = src.alignment_bytes;
                }
            }
        }

        void adopt_backend(SimdBackend& dst, const SimdBackend& candidate)
        {
            adopt_op(dst.add, candidate.add);
            adopt_op(dst.multiply, candidate.multiply);
            adopt_op(dst.multiply_scalar, candidate.multiply_scalar);
            adopt_op(dst.multiply_add, candidate.multiply_add);
            adopt_op(dst.multiply_scalar_add, candidate.multiply_scalar_add);
        }

#if defined(__x86_64__)
        SimdBackend make_avx_backend()
        {
            SimdBackend backend{};
            if (!__builtin_cpu_supports("avx"))
            {
                return backend;
            }

            constexpr int32_t avx_float_block = 8;
            constexpr std::size_t avx_alignment = 32;

            backend.add = make_op(x86::fadd_avx, avx_float_block, avx_alignment);
            backend.multiply = make_op(x86::fmul_avx, avx_float_block, avx_alignment);
            backend.multiply_scalar = make_op(x86::fmulc_avx, avx_float_block, avx_alignment);
            backend.multiply_add = make_op(x86::fmuladd_avx, avx_float_block, avx_alignment);
            backend.multiply_scalar_add = make_op(x86::fmulcadd_avx, avx_float_block, avx_alignment);
            return backend;
        }

        SimdBackend make_fma_backend()
        {
            SimdBackend backend{};
            if (!__builtin_cpu_supports("fma"))
            {
                return backend;
            }

            constexpr int32_t avx_float_block = 8;
            constexpr std::size_t avx_alignment = 32;
            backend.multiply_add = make_op(x86::fmuladd_fma, avx_float_block, avx_alignment);
            backend.multiply_scalar_add = make_op(x86::fmulcadd_fma, avx_float_block, avx_alignment);
            return backend;
        }
#endif
    } // namespace

    SimdRuntime::SimdRuntime()
    {
        backend_ = make_scalar_backend();

#if defined(__x86_64__)
        __builtin_cpu_init();
        adopt_backend(backend_, make_avx_backend());
        adopt_backend(backend_, make_fma_backend());
#endif
    }

    const SimdBackend& backend()
    {
        return SimdRuntime::instance().backend();
    }

} // namespace detail

namespace
{
    std::size_t align_to_block(const std::size_t value, const int32_t block_size)
    {
        const std::size_t block = block_size > 0 ? static_cast<std::size_t>(block_size) : 1;
        return value - (value % block);
    }

    template <typename Op, typename SimdInvoker, typename ScalarInvoker>
    void run_with_backend(const Op& op, std::size_t size, bool aligned, SimdInvoker&& simd_call, ScalarInvoker&& scalar_call)
    {
        if (size == 0)
        {
            return;
        }

        const bool has_simd = static_cast<bool>(op);
        const std::size_t simd_size = has_simd ? align_to_block(size, op.block) : 0;

        if (has_simd && simd_size > 0)
        {
            simd_call(simd_size, aligned);
        }

        const std::size_t remainder = size - simd_size;
        if (!has_simd || remainder > 0)
        {
            scalar_call(simd_size, remainder);
        }
    }
}

void vector_add(const float* InA, const float* InB, float* Out, std::size_t Sz, std::size_t alignment_bytes)
{
    const auto& op = detail::backend().add;
    const bool use_aligned = alignment_bytes >= op.alignment_bytes;
    run_with_backend(op, Sz, use_aligned,
        [&](std::size_t block, bool aligned) { op.fn(InA, InB, Out, block, aligned); },
        [&](std::size_t offset, std::size_t len)
        {
            if (len == 0) return;
            detail::scalar::fadd_scalar(InA + offset, InB + offset, Out + offset, len);
        });
}

void vector_multiply(const float* InA, const float* InB, float* Out, std::size_t Sz, std::size_t alignment_bytes)
{
    const auto& op = detail::backend().multiply;
    const bool use_aligned = alignment_bytes >= op.alignment_bytes;
    run_with_backend(op, Sz, use_aligned,
        [&](std::size_t block, bool aligned) { op.fn(InA, InB, Out, block, aligned); },
        [&](std::size_t offset, std::size_t len)
        {
            if (len == 0) return;
            detail::scalar::fmul_scalar(InA + offset, InB + offset, Out + offset, len);
        });
}

void vector_multiply_scalar(const float* In, const float& k, float* Out, std::size_t Sz, std::size_t alignment_bytes)
{
    const auto& op = detail::backend().multiply_scalar;
    const bool use_aligned = alignment_bytes >= op.alignment_bytes;
    run_with_backend(op, Sz, use_aligned,
        [&](std::size_t block, bool aligned) { op.fn(In, &k, Out, block, aligned); },
        [&](std::size_t offset, std::size_t len)
        {
            if (len == 0) return;
            detail::scalar::fmulc_scalar(In + offset, &k, Out + offset, len);
        });
}

void vector_multiply_add(const float* InMulA, const float* InMulB, const float* InAddC, float* Out, std::size_t Sz, std::size_t alignment_bytes)
{
    const auto& op = detail::backend().multiply_add;
    const bool use_aligned = alignment_bytes >= op.alignment_bytes;
    run_with_backend(op, Sz, use_aligned,
        [&](std::size_t block, bool aligned) { op.fn(InMulA, InMulB, InAddC, Out, block, aligned); },
        [&](std::size_t offset, std::size_t len)
        {
            if (len == 0) return;
            detail::scalar::fmuladd_scalar(InMulA + offset, InMulB + offset, InAddC + offset, Out + offset, len);
        });
}

void vector_multiply_scalar_add(const float* InMul, const float& k, const float* InAdd, float* Out, std::size_t Sz, std::size_t alignment_bytes)
{
    const auto& op = detail::backend().multiply_scalar_add;
    const bool use_aligned = alignment_bytes >= op.alignment_bytes;
    run_with_backend(op, Sz, use_aligned,
        [&](std::size_t block, bool aligned) { op.fn(InMul, &k, InAdd, Out, block, aligned); },
        [&](std::size_t offset, std::size_t len)
        {
            if (len == 0) return;
            detail::scalar::fmulcadd_scalar(InMul + offset, &k, InAdd + offset, Out + offset, len);
        });
}

} // namespace vms::dsp
