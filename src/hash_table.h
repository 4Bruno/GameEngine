#ifndef GAME_HASHING
#define GAME_HASHING


#include "game_platform.h"
#include "game_memory.h"

#define HASH_TABLE_MAP_FUNCTION(name) u32 name(void * Data)
typedef HASH_TABLE_MAP_FUNCTION(hash_table_map_function);

#define HASH_TABLE_COMPARE_FUNCTION(name) i32 name(void * Entry, void * EntryLookup)
typedef HASH_TABLE_COMPARE_FUNCTION(hash_table_compare_function);

struct bucket
{
    bucket * Next;

    u32      EntriesCount;
    void   * Entries;
};

struct hash_table
{
    bucket * Buckets;
    u32      BucketCount;
    bucket * FreeList;

    memory_arena * Arena;

    u32 EntrySize;
    u32 EntriesPerBucket;
    
    hash_table_map_function     * pfnHashMap;
    hash_table_compare_function * pfnCompareEntry;

    u32    CountLookups;
    u32    CostFinding;
};

void * 
AddEntryToTable(hash_table * Table, void * DataEntry);

#define InitializeHashTable(Arena, type, EntriesPerBucket, pfnHashMap, pfnCompare, BucktCount) \
        InitializeHashTable_(Arena, sizeof(type), EntriesPerBucket, pfnHashMap, pfnCompare, BucktCount)

hash_table
InitializeHashTable_(memory_arena * Arena, 
                     u32 EntrySize, u32 EntriesPerBucket, 
                     hash_table_map_function     * pfnHashMap,
                     hash_table_compare_function * pfnCompare,
                     u32 DefaultBucketCount = 4096);
b32
RemoveEntryFromTable(hash_table * Table, void * DataEntry);

#endif
