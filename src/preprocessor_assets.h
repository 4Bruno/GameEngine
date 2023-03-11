#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <windows.h>
#include <stdint.h>
#include <stdio.h> // printf

typedef int16_t  i16;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  b32;
typedef float    r32;
typedef double   r64;

#define Log(format, ...) printf(format, __VA_ARGS__)
#define Logn(format, ...) printf(format "\n", __VA_ARGS__)
#define Assert(exp) if (!(exp)) { Logn("------------- FAILED ASSERTION -------------\nFile:%s\nLine: %i\nExpc:%s",__FILE__,__LINE__,#exp);*(volatile int *)0 = 0; } 

#define Kilobytes(x) x*1024
#define Megabytes(x) x*1024*Kilobytes(1)
#define Gigabytes(x) x*1024*Megabytes(1)


#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

struct memory_arena
{
    u8 * Base;
    u32 MaxSize;
    u32 CurrentSize;

    // Arena might or not be used as temp memory
    // this will be used by begin/end temparena
    u32 StackTemporaryMemory;
};

struct thread_memory_arena
{
    b32 InUse;
    memory_arena Arena;
};

struct memory_aligned_result
{
    uintptr_t AddressAligned;
    u32 Delta;
};


#define PushSize(Arena,Size) _PushSize(Arena,Size*sizeof(char))
#define PushArray(Arena,Count,Struct) (Struct *)_PushSize(Arena,Count*sizeof(Struct))
#define PushStruct(Arena,Struct) (Struct *)PushArray(Arena,1,Struct)

#define BeginTempArena(Arena,ID) u32 Arena##SizeBegin##ID = (Arena)->CurrentSize;\
                                 ++Arena->StackTemporaryMemory;
#define EndTempArena(Arena,ID) --Arena->StackTemporaryMemory;\
                               Arena->CurrentSize = Arena##SizeBegin##ID;
#define TempArenaSanityCheck(Arena) Assert(Arena->StackTemporaryMemory == 0)

inline u8 *
_PushSize(memory_arena * Arena,u32 Size)
{
    Assert((Arena->CurrentSize + Size) <= Arena->MaxSize);
    u8 * BaseAddr = Arena->Base + Arena->CurrentSize;
    Arena->CurrentSize += Size;
    return BaseAddr;
}

inline void
InitializeArena(memory_arena * Arena,u8 * BaseAddr, u32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

#endif
