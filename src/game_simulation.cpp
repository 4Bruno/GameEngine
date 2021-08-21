#include "game.h"

#if 0
inline b32
IsSlotOccupied(simulation * Sim, u32 Slot)
{
    // ie: 13432 / 64 where 64 is size of system ptr
    // bucket where bit can be found
    u32 Bucket = (Slot * SIMULATION_ONE_OVER_ENTRY_BUCKET_SIZE);
    // ie: 13432 % 64 bit to flip
    u32 Index = Slot % sizeof(entry_bucket);

    intptr_t BitArray = Sim->EntityEntriesOccupancy[Slot];

    b32 IsOccupaid = BitArray & ((entry_bucket)1 << Index);

    return IsOccupaid;
}

inline b32
IsSlotFree(simulation * Sim, u32 Slot)
{
    return IsSlotOccupied(Sim, Slot);
}
#endif

sparse_entry *
GetNextAvailableEntry(simulation * Sim)
{
    Assert(Sim->EntryCount < ArrayCount(Sim->EntityEntries) - 1);
    u32 SlotIndex = Sim->EntryCount++;
    sparse_entry * Entry = Sim->EntityEntries + SlotIndex;

    return Entry;
}

simulation_iterator
BeginSimIterator(world * World, simulation * Sim)
{
    simulation_iterator Iterator;
    Iterator.Index  = 0; // u32   Index;
    Iterator.Entity = 0;
    Iterator.Sim = Sim;
    Iterator.World = World;
    if (Sim->EntryCount > 0)
    {
        u32 StorageIndex = Sim->EntityEntries[0].StorageIndex;
        Iterator.Entity = World->ActiveEntities + StorageIndex;
    }

    return Iterator;
}

entity *
AdvanceSimIterator(simulation_iterator * Iterator)
{
    if (Iterator->Index < (Iterator->Sim->EntryCount -1))
    {
        u32 StorageIndex = Iterator->Sim->EntityEntries[++Iterator->Index].StorageIndex;
        Iterator->Entity = Iterator->World->ActiveEntities + StorageIndex;
    }
    else
    {
        Iterator->Entity = 0;
    }

    return Iterator->Entity;
}

void
SimulationRegisterEntity(simulation * Sim, entity * Entity,  u32 StorageIndexID)
{
    Assert(Sim->EntryCount < ArrayCount(Sim->EntityEntries));
    sparse_entry * Entry = GetNextAvailableEntry(Sim);
    Entry->StorageIndex = StorageIndexID;

#if 0
    if (EntityHasFlag(Entity,component_transform))
    {
        Assert(Sim->TransformCount < MAX_WORLD_ENTITY_COUNT_MINUS_ONE)
        sparse_entry * EntryT = Sim->TransformEntries + Sim->TransformCount++;
        EntryT->StorageIndex = StorageIndexID;
    }
#endif
}

// TODO: own memory for sorting so can be multithreaded
// not multi-thread
void
SortEntities(simulation * Sim,sparse_entry * Entries, u32 EntryCount)
{
    Assert(EntryCount < MAX_WORLD_ENTITY_COUNT_MINUS_ONE);

    u32 * C = Sim->SortingBuckets;

    for (u32 EntryIndex = 0;
                EntryIndex < EntryCount;
                ++EntryIndex)
    {
        ++C[Entries[EntryIndex].StorageIndex];
    }

    u32 Current = 0;
    for (u32 CountIndex = 0;
                CountIndex < MAX_WORLD_ENTITY_COUNT_MINUS_ONE;
                ++CountIndex)
    {
        for (u32 Offset = 0; 
                    Offset < C[CountIndex];
                    ++Offset)
        {
            Entries[Current++].StorageIndex = CountIndex;
        }
    }
}

void
BeginSimulation(world * World, simulation * Sim)
{
    Sim->EntryCount = 0;
    Sim->TransformCount = 0;

    UpdateWorldLocation(World, Sim);

    sparse_entry * Entries = Sim->EntityEntries;
    u32 Last = 0;
    for (u32 EntryIndex = 0;
                EntryIndex < Sim->EntryCount;
                ++EntryIndex)
    {
        u32 Current = Entries[EntryIndex].StorageIndex;
        if (Last > Current)
        {
            SortEntities(Sim, Sim->TransformEntries, Sim->TransformCount);
            break;
        }
        Last = Current;
    }
}


