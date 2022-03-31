#ifndef GAME_RANDOM_H
#define GAME_RANDOM_H

#include <limits.h>

struct f32_4x
{
    union
    {
        __m128 P;
        r32 E[4];
        u32 U32[4];
    };
};

inline f32_4x
F32_4x(__m128 m128)
{
    f32_4x Result;

    Result.P = m128;

    return Result;
}

inline f32_4x
U32_4x(u32 E0, u32 E1, u32 E2, u32 E3)
{
    f32_4x Result;

    Result.P = _mm_setr_ps(*(float *)&E3,*(float *)&E2,*(float *)&E1,*(float *)&E0);

    return Result;
}

#define SHIFT_LEFT_4X(f32x4,N)  F32_4x(_mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(f32x4.P),N)))
#define SHIFT_RIGHT_4X(f32x4,N) F32_4x(_mm_castsi128_ps(_mm_srli_epi32(_mm_castps_si128(f32x4.P),N)))

inline f32_4x
operator ^(f32_4x LS, f32_4x RS)
{
    f32_4x Result;

    Result.P = _mm_xor_ps(LS.P,RS.P);

    return Result;
}

inline f32_4x &
operator ^=(f32_4x &LS, f32_4x RS)
{
    LS = LS ^ RS;

    return LS;
}


struct random_series
{
    f32_4x State;    
};

inline random_series 
RandomSeed(u32 E0 = 74561346, u32 E1 = 451676, u32 E2 = 88766931, u32 E3 = 3448765)
{
    random_series Series;

    Series.State = U32_4x(E0,E1,E2,E3);

    return Series;
}

inline f32_4x 
RandomNextU324X(random_series * Series)
{
    f32_4x Result = Series->State;

    Result ^= SHIFT_LEFT_4X(Result, 13);
    Result ^= SHIFT_RIGHT_4X(Result, 17);
    Result ^= SHIFT_LEFT_4X(Result, 5);

    Series->State = Result;
    
    return(Result);
}

inline u32 
RandomNextU32(random_series * Series)
{
    u32 Result;

    Result = RandomNextU324X(Series).U32[0];

    return Result;
}

inline r32
RandomUnilateral(random_series * Series)
{
    r32 D = 1.0f / (r32)UINT32_MAX;

    r32 Result = (r32)RandomNextU32(Series) * D;

    return Result;
}
inline r32
RandomBilateral(random_series * Series)
{
    r32 Result = RandomUnilateral(Series) * 2.0f - 1.0f;

    return Result;
}

inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t) * A + t*B;

    return Result;
}

inline r32
RandomBetween(random_series * Series, r32 Min, r32 Max)
{
    r32 Result = Lerp(Min,RandomUnilateral(Series),Max);

    return Result;
}


#endif
