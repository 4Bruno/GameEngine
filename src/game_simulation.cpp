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
GetNextAvailableEntry(simulation * Sim)
{
    Assert(Sim->EntryCount < ArrayCount(Sim->EntityEntries) - 1);
    u32 SlotIndex = Sim->EntryCount++;
    sparse_entry * Entry = Sim->EntityEntries + SlotIndex;

    return Entry;
}
simulation_mesh_obj_transform_iterator
BeginSimMeshObjTransformIterator(simulation * Sim, entity * Entity)
{
    simulation_mesh_obj_transform_iterator Iter;
    Iter.Index = 0;
    Iter.T = 0;
    Iter.MeshObjCount = Entity->MeshObjCount;
    if (Entity->MeshObjCount > 1)
    {
        if (IS_VALID_MESHOBJ_TRANSFORM_INDEX(Entity->MeshObjTransOcuppancyIndex))
        {
            u32 IndexStart = Sim->MeshObjTransformID[Entity->MeshObjTransOcuppancyIndex];
            entity_transform * T = Sim->MeshObjTransform + IndexStart;
            Iter.T = T;
        }
        else
        {
            Iter.T = 0;
        }
    }
    else
    {
        Iter.T = &Entity->Transform;
    }
    return Iter;
}
entity_transform *
AdvanceSimMeshObjTransformIterator(simulation_mesh_obj_transform_iterator * Iter)
{
    ++Iter->Index;
    if (Iter->Index < Iter->MeshObjCount)
    {
        ++Iter->T;
    }
    else
    {
        Iter->T = 0;
    }
    return Iter->T;
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
        Logn("Unregistered Entity %i",Entity->ID.ID);
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

void
SimulationRegisterEntity(simulation * Sim, entity * Entity,  u32 StorageIndexID)
{
    Assert(Sim->EntryCount < ArrayCount(Sim->EntityEntries));
    sparse_entry * Entry = GetNextAvailableEntry(Sim);
    Entry->StorageIndex = StorageIndexID;

    /*
     * For entities with complex meshes (multiple objects).
     * Each object within the mesh has local transform matrix.
     * This data lives in a temporary array in the simulation.
     * Consequence, once unregistered, we lose object transforms. I'm OK.
     * Every time we load such a mesh, we assign specific bucket which
     * storages the start index in the temporary array where all the 
     * transforms for each object within the mesh.
     * The bucket is inmutable, it is an interface pointed by entities.
     * Where the bucket points is local to the simulation and can change
     * if is convenient (ie: re-order/clean the array). 
     */
    if (
            (Entity->MeshObjCount > 1) &&
            IS_VALID_MESHID(Entity->MeshID.ID) &&
            (!IS_VALID_MESHOBJ_TRANSFORM_INDEX(Entity->MeshObjTransOcuppancyIndex))
       )
    {
        u32 BucketsCount = Entity->MeshObjCount;

        u32 BucketIndex = FindAndSetFirstAvailableBucket(Sim,BucketsCount);

        Assert(IS_VALID_MESHOBJ_TRANSFORM_INDEX(BucketIndex));

        u32 * Bucket = Sim->MeshObjTransformID + BucketIndex;

        Entity->MeshObjTransOcuppancyIndex = BucketIndex;

        // TODO:  if this fails, reconstruct array to free buckets
        //        then check again if still has no space
        Assert((Sim->MeshObjTransformCount + BucketsCount) < ArrayCount(Sim->MeshObjTransform));

        // points to the first available index will be used
        *Bucket = Sim->MeshObjTransformCount;

        mesh_group Mesh = GetMeshInfo(Entity->MeshID);

        for (u32 MeshIndex = 0;
                MeshIndex < Mesh.TotalMeshObjects;
                ++MeshIndex)
        {
            entity_transform * T = (Sim->MeshObjTransform + Sim->MeshObjTransformCount++);
            v3 Scale = V3(1,1,1);
            T->LocalS = Scale;
            Quaternion_setIdentity(&T->LocalR);
            Translate(T->WorldP,T->LocalP);
            T->WorldS = Scale;
            T->WorldR = T->LocalR;
            T->WorldT = {};
        }

        Logn("Registered Entity with multiple mesh objects %i",Entity->ID.ID);
    }

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


