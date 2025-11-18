/*
    Library Utilities - Copyright (C) 2025 Manuel Virgilio
    This file is part of a project licensed under the terms
    of the LGPLv3 + Attribution. See LICENSE for details.
*/

#include <vms/dsp/simd_default.h>

#include <cmath>

namespace vms::dsp::detail::scalar
{

void fmuladd_scalar(const float* MulA, const float*  MulB, const float* AddC, float* Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (MulA[i] * MulB[i]) + AddC[i];
    }
}

void fmulcadd_scalar(const float* Mul, const float*  Const, const float* AddC, float* Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (Mul[i] * *Const) + AddC[i];
    }
}

void fadd_scalar(const float* AddA, const float*  AddB, float* Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (AddA[i] + AddB[i]);
    }
}

void fsub_scalar(const float *SubA, const float *SubB, float *Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (SubA[i] - SubB[i]);
    }
}

void fdsub_scalar(const double *SubA, const double *SubB, double *Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (SubA[i] - SubB[i]);
    }
}

void fmulc_scalar(const float* Mul, const float*  Const, float* Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (Mul[i] * *Const);
    }
}

void fmul_scalar(const float* MulA, const float* MulB, float* Res, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Res[i] = (MulA[i] * MulB[i]);
    }
}

void fceil_scalar(const float *In, float *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = std::ceil(In[i]);
    }
}

void ffloor_scalar(const float *In, float *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = std::floor(In[i]);
    }
}

void fdceil_scalar(const double *In, double *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = std::ceil(In[i]);
    }
}

void fdfloor_scalar(const double *In, double *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = std::floor(In[i]);
    }
}

void f_to_int32_scalar(const float *In, int32_t *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = static_cast<int32_t>(In[i]);
    }
}

void fd_to_int32_scalar(const double *In, int32_t *Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = static_cast<int32_t>(In[i]);
    }
}

void flerp_scalar(const int32_t* InV0, const int32_t* InV1, const float* InT, const float* In, float* Out, const std::size_t len)
{
    for ( std::size_t i = 0 ; i < len ; ++i )
    {
        Out[i] = (1.f - InT[i]) * In[InV0[i]] + InT[i] * In[InV1[i]];
    }
}

} // namespace vms::dsp::detail::scalar
