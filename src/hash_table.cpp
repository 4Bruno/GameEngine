#include "hash_table.h"
#include "game_math.h"
#include "game_memory.h"


u32
GetHashFromID(hash_table * Table, void * Data)
{
    u32 Result = Table->pfnHashMap(Data);

    return Result;
}

b32
RemoveEntryFromTable(hash_table * Table, void * DataEntry)
{
    b32 RemovedSuccessfuly = false;

    u32 HashKey = GetHashFromID(Table, DataEntry);

    u32 BucketIndex = HashKey & (Table->BucketCount - 1);

    bucket  * Bucket = Table->Buckets + BucketIndex;
    bucket ** Parent = &(Table->Buckets);

    while (Bucket && Bucket->EntriesCount > 0)
    {
        for (u32 EntryIndex = 0;
                EntryIndex < Bucket->EntriesCount;
                ++EntryIndex)
        {
            void * Entry = ((u8 *)Bucket->Entries + (EntryIndex * Table->EntrySize));
            if (Table->pfnCompareEntry(Entry, DataEntry) == 0)
            {
                void * LastEntry = 
                    (void * )((u8 *)Bucket->Entries + (Bucket->EntriesCount * Table->EntrySize));
                if (LastEntry != Entry)
                {
                    memcpy(Entry, LastEntry, Table->EntrySize);
                }
                --Bucket->EntriesCount;
                if (Bucket->EntriesCount == 0)
                {
                    *Parent = Bucket->Next;
                    Bucket->Next = Table->FreeList;
                    Table->FreeList = Bucket;
                }
                RemovedSuccessfuly = true;
                break;
            }
        }
        if (RemovedSuccessfuly) 
        {
            break;
        }
        Bucket = Bucket->Next;
        Parent = &Bucket->Next;
    }

    return RemovedSuccessfuly;
}

void * 
AddEntryToTable(hash_table * Table, void * DataEntry)
{
    u32 HashKey = GetHashFromID(Table, DataEntry);

    u32 BucketIndex = HashKey & (Table->BucketCount - 1);

    bucket  * Bucket = Table->Buckets + BucketIndex;
    bucket  * Parent = Table->Buckets;

    while (Bucket && (Bucket->EntriesCount == Table->EntriesPerBucket))
    {
        Parent = Bucket;
        Bucket = Bucket->Next;
    }

    if (!Bucket)
    {
        if (Table->FreeList)
        {
            Bucket          = Table->FreeList;
            Table->FreeList = Bucket->Next;
        }
        else
        {
            Bucket          = PushStruct(Table->Arena, bucket);
            Bucket->Entries = 0;
        }

        if (!Bucket->Entries)
        {
            Bucket->Entries = PushSize(Table->Arena, Table->EntriesPerBucket * Table->EntrySize);
        }

        Assert(Bucket);
        Bucket->EntriesCount = 0;
        Bucket->Next = 0;
        
        Parent->Next = Bucket;
    }

    void * Entry = (u8 *)Bucket->Entries + (Bucket->EntriesCount++ * Table->EntrySize);

    return Entry;
}

hash_table
InitializeHashTable_(memory_arena * Arena, 
                     u32 EntrySize, u32 EntriesPerBucket, 
                     hash_table_map_function     * pfnHashMap,
                     hash_table_compare_function * pfnCompare,
                     u32 DefaultBucketCount)
{

    Assert(pfnHashMap);
    Assert(pfnCompare);
    Assert(EntrySize > 0);
    Assert(Arena);

    hash_table Table;
    
    u32 BucketCount = NextPowerOf2(DefaultBucketCount);

    Table.Buckets          = PushArray(Arena, BucketCount, bucket);
    Table.BucketCount      = BucketCount;
    Table.pfnHashMap       = pfnHashMap;
    Table.pfnCompareEntry  = pfnCompare;
    Table.EntrySize        = EntrySize;
    Table.EntriesPerBucket = EntriesPerBucket;
    Table.Arena            = Arena;
    Table.FreeList         = 0;

    return Table;
}
