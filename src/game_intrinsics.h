#ifndef GAME_INTRINSICS_H
#define GAME_INTRINSICS_H

#include "game_platform.h"

#if _MSC_VER
#include "windows.h"
inline u32 AtomicCompareExchangeU32(volatile u32 * This, u32 UpdateTo, u32 IfMatchesThis)
{
    u32 ThisValueBeforeExchange =
        InterlockedCompareExchange(
            (u32 volatile *)This,
            UpdateTo,
            IfMatchesThis);

    return ThisValueBeforeExchange;
}

inline u32 AtomicExchangeU32(volatile u32 * This, u32 UpdateTo)
{
    u32 ThisValueBeforeExchange =
        InterlockedExchange(
            (u32 volatile *)This,
            UpdateTo);

    return ThisValueBeforeExchange;
}
#endif



#endif
