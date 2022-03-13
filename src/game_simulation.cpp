#include "game.h"
#include "game_mesh.h"


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
GetNextAvailableGroundEntry(simulation * Sim)
{
    Assert(Sim->GroundEntryCount <= ArrayCount(Sim->GroundEntries));
    u32 SlotIndex = Sim->GroundEntryCount++;
    sparse_entry * Entry = Sim->GroundEntries + SlotIndex;

    return Entry;
}
sparse_entry *
GetNextAvailableEntry(simulation * Sim)
{
    Assert(Sim->EntryCount < ArrayCount(Sim->EntityEntries));
    u32 SlotIndex = Sim->EntryCount++;
    sparse_entry * Entry = Sim->EntityEntries + SlotIndex;

    return Entry;
}


simulation_iterator
BeginSimGroundIterator(world * World, simulation * Sim)
{
    simulation_iterator Iterator;
    Iterator.Index  = 0; // u32   Index;
    Iterator.Entity = 0;
    Iterator.Sim = Sim;
    Iterator.World = World;
    if (Sim->GroundEntryCount > 0)
    {
        u32 StorageIndex = Sim->GroundEntries[0].StorageIndex;
        Iterator.Entity = World->GroundEntities + StorageIndex;
    }

    return Iterator;
}

entity *
AdvanceSimGroundIterator(simulation_iterator * Iterator)
{
    if (Iterator->Index < (Iterator->Sim->GroundEntryCount -1))
    {
        u32 StorageIndex = Iterator->Sim->GroundEntries[++Iterator->Index].StorageIndex;
        Iterator->Entity = Iterator->World->GroundEntities + StorageIndex;
    }
    else
    {
        Iterator->Entity = 0;
    }

    return Iterator->Entity;
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

#if 0
void
SimulationUnregisterEntity(simulation * Sim, entity * Entity,  u32 StorageIndexID)
{
    /*
     * Every frame everything is unregistered technically.
     * This is a special call when entity truly goes out of scope (no room or too far)
     * so we can release occupancy index for the mesh objects
     */
    if (IS_VALID_MESHOBJ_TRANSFORM_INDEX(Entity->MeshObjTransOcuppancyIndex))
    {
        /* I got this wrong and I thought I had to set multiple bits for each mesh object
         * but that is not necessary. Now instead is only 1
         * I leave code as is which can actually handle setting bits from multiple uintptr_t
         * for as many objects. Just tweaked to set objects to 1
         */
        //u32 CountObjects = Entity->MeshObjCount;
        u32 CountObjects = 1;
        r32 RealUinptrArray = Entity->MeshObjTransOcuppancyIndex * (1.0f / sizeof(uintptr_t));
        // index = 60 and 80 objects
        // 60 - 64 = 5 to allocate 1st 64
        // 1 - 64 = 64 to allocated 2nd 64
        // 1 - 11 = 11 to allocated 3rd 64
        u32 IndexAccessor = (u32)RealUinptrArray;
        u32 IndexBlock = (u32)((RealUinptrArray - IndexAccessor) * sizeof(uintptr_t)) - 1;

        uintptr_t * OccuppancyIndexStart = Sim->MeshObjTransformOccupancy + IndexAccessor;
        while (CountObjects > 0)
        {
            // 1) (64 - 59) = 5 buckets
            u32 Allowance = (sizeof(uintptr_t) - IndexBlock);
            // 1) 80 - 5 = 75 remaining objects
            u32 ObjectsToSetZero = CountObjects - Allowance;
            //CountObjects -= ObjectsToSetZero;
            CountObjects -= 1;
            // (1 << 5) - 1
            // 1) 0x0000000000000000000000000000000000000000000000000000000000011111
            // ((1 << 5) - 1) << 59 
            // 1) 0x1111100000000000000000000000000000000000000000000000000000000000
            // (((1 << 5) - 1) << 59) & u64
            //uintptr_t Mask = (1 << ObjectsToSetZero) - 1;
            uintptr_t Mask = ~((uintptr_t)0x01 << IndexBlock);
            *OccuppancyIndexStart = *OccuppancyIndexStart & Mask;
            ++OccuppancyIndexStart;
            IndexBlock = 1; // here after always 1
        }
        Entity->MeshObjTransOcuppancyIndex = INVALID_MESHOBJ_TRANSFORM_INDEX;
        //Logn("Unregistered Entity %i",Entity->ID.ID);
    }
}


u32
FindAndSetFirstAvailableBucket(simulation * Sim,u32 BucketsCount)
{
    u32 BucketBitIndex = INVALID_MESHOBJ_TRANSFORM_INDEX;

    for (u32 OccuppancyBucketIndex = 0;
                OccuppancyBucketIndex < ArrayCount(Sim->MeshObjTransformOccupancy);
                ++OccuppancyBucketIndex)
    {
        uintptr_t * Bucket = Sim->MeshObjTransformOccupancy + OccuppancyBucketIndex;
        unsigned long Index;
        // flip bits then find first set
        if (_BitScanForward64(&Index,~(*Bucket)) != 0)
        {
            uintptr_t Mask = ((uintptr_t)0x01 << Index);
            *Bucket = *Bucket | Mask;
            BucketBitIndex = (OccuppancyBucketIndex * sizeof(uintptr_t)) + Index;
            break;
        }
    }

    return BucketBitIndex;
}
#endif

void
SimulationRegisterGround(simulation * Sim,entity * Ground,u32 GroundIndex)
{
    Assert(Sim->GroundEntryCount < ArrayCount(Sim->GroundEntries));
    sparse_entry * Entry = GetNextAvailableGroundEntry(Sim);
    Entry->StorageIndex = GroundIndex;
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
    START_CYCLE_COUNT(begin_simulation);

    Sim->EntryCount = 0;
    Sim->TransformCount = 0;
    Sim->GroundEntryCount = 0;

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

    END_CYCLE_COUNT(begin_simulation);
}


