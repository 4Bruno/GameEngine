#ifndef GAME_WORLD_H
#define GAME_WORLD_H
/*
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 */

#include "game_platform.h"
#include "game_memory.h"
#include "Quaternion.h"
#include "game_math.h"
#include "game_mesh.h"
#include <stddef.h>
#include "game_assets.h"

#define MAX_WORLD_ENTITY_COUNT (1 << 14)
#define MAX_WORLD_ENTITY_COUNT_MINUS_ONE (MAX_WORLD_ENTITY_COUNT - 1)
#define MAX_WORLD_GROUND_COUNT (64)

// Helpers to printf world positions
#define STRP "fx:%f fy:%f fz:%f"
#define STRWORLDP "X:%i Y:%i Z:%i " STRP
#define FWORLDP(P) P.x, P.y, P.z, P._Offset.x, P._Offset.y, P._Offset.z
#define FP(P) P.x, P.y, P.z

// game_entity.h

enum component_flags
{
    component_none = 0 << 0,

    component_input     = 1 << 0,
    component_collision = 1 << 1,
    component_render_3d = 1 << 2,
    component_transform = 1 << 3,
    component_momentum  = 1 << 4,
    component_ground    = 1 << 5,
    component_delete    = 1 << 6

};

struct sparse_entry
{
    u32 StorageIndex;
};

struct world_pos
{
    // Those are x,y,z positions in the world grid
    // using float allows for a 4 Kilometers world
    // (where objects are size of 0.02 mm - grand of sand)
    // by using u32 x/y/z we have more size
    i32 x, y ,z;
    // Within our grid cell, float precission offset
    v3 _Offset;
};

struct entity_input
{
    v3 dP;
    v3 EulerXYZ;
};


struct entity_transform
{
    v3 LocalP; // 12
    v3 LocalS; // 24
    Quaternion LocalR; // 40

    r32 Yaw, Pitch; // 48

    m4 WorldP; // 12 * 4 = 48 + 48 = 96
    v3 WorldS; // 108
    Quaternion WorldR; // 122

    m4 WorldT; // 122 + 48 = 170
};

struct entity_id
{
    u32 ID;
};

struct entity
{
    // Global identifier. Must be unique
    entity_id ID;    // 4
    // Position relative to the world center
    world_pos WorldP; // 28

    component_flags Flags; // 32

    u32 Height;

    game_asset_id MeshID;
    u32 MeshObjTransOcuppancyIndex;

    game_asset_id Material;
    v3 Color; // 36
    r32 Transparency;
    game_asset_id TextureID;

    entity_transform Transform; // 170 + 36 = 206

    /* 
     * DATA TO NOT TO STORAGE IN HASH GRID 
     * This is only useful for active entities
     * 
     */
    u8 DO_NOT_COPY_PAST_THIS_POINT;
    world_pos GroundChunkP;
};


/*
 * Large buffer to accommodate a big number of entities data
 * if cell needs to add more data and is not sufficient
 * it uses linked list to add new chunk of data
 */
#define WORLD_CELL_ENTITY_COUNT 5 
#define WORLD_CELL_ENTITY_SIZE (sizeof(entity) - (sizeof(entity) - offsetof(entity,DO_NOT_COPY_PAST_THIS_POINT)))
#define WORLD_CELL_DATA_SIZE (WORLD_CELL_ENTITY_SIZE * WORLD_CELL_ENTITY_COUNT)
struct world_cell_data
{
    u16 DataSize;
    u8 Data[WORLD_CELL_DATA_SIZE];

    world_cell_data * Next;
};

struct cell_neighbor_offset
{
    // In 3D any cell has 26 cells around + 1 (self)
    i32 Offset[27];
};

/*
 * cells for a given x,y,z only exists as long as there are entities on it
 * they are to be removed otherwise and push back to a free list
 */
struct world_cell
{
    // uniquely identifies cell
    i32 x, y , z;
    u32 HashIndex;

    cell_neighbor_offset * Neighbor;

    // For this cell, all data chunks associated as a linked list
    world_cell_data * FirstCellData;

    world_cell * NextCell;
};


// In a simulation, how many entities can have multiple objects associated
// to the mesh
#define SIMULATION_TEMP_MESHOBJ_TRANSFORM (2 << 10)
#define SIMULATION_AVG_MESH_OBJS 10

struct simulation
{
    world_pos Origin;    
    v3 Dim;
    
    // sparse array to world entities access
    sparse_entry EntityEntries[MAX_WORLD_ENTITY_COUNT];
    u32 EntryCount;

    sparse_entry GroundEntries[MAX_WORLD_GROUND_COUNT];
    u32 GroundEntryCount;

    sparse_entry TransformEntries[MAX_WORLD_ENTITY_COUNT];
    u32 TransformCount;

#if 0
    u32 MeshObjTransformCount;
    u32 MeshObjTransLastAvailableIndex;
    entity_transform MeshObjTransform[SIMULATION_TEMP_MESHOBJ_TRANSFORM*SIMULATION_AVG_MESH_OBJS];
    // Index of MeshObjTransform where it begins
    u32 MeshObjTransformID[SIMULATION_TEMP_MESHOBJ_TRANSFORM];
    // in 64 bit system:
    // buckets / array of 64bits
    // we signal them to 1 if in use, 0 otherwise
    uintptr_t MeshObjTransformOccupancy[(SIMULATION_TEMP_MESHOBJ_TRANSFORM / sizeof(uintptr_t))];
#endif

    u32 SortingBuckets[MAX_WORLD_ENTITY_COUNT];
    
};

/*
 * Spatial partition:
 * uniform grid open hashing
 * Christer Ericson Chapter 7
 * Implementation based on other paper:
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 *
 * World has:
 * - HashGrid 
 *   For storaging world data
 * - ActiveEntities
 *   Cached (duplicated) data for entities unpacked
 */
struct world
{
    // arbitrary for memory/speed trade-off
    // MUST be power of 2 - preferred 16
    v3 GridCellDimInMeters;
    v3 OneOverGridCellDimInMeters;

    // used for unique entity ID
    u32 TotalWorldEntities;

    // Grid dimensions
    u32 HashGridX,HashGridY,HashGridZ;
    u32 HashGridXMinusOne,HashGridYMinusOne,HashGridZMinusOne;

    u32 HashGridSizeMinusOne;
    world_cell ** HashGrid;

    // Constant for all cells not in edge
    cell_neighbor_offset InnerNeighbors;
    // Based on Grid size
    cell_neighbor_offset * OuterNeighbors;
    
    world_cell      * FreeListWorldCells;
    world_cell_data * FreeListWorldCellData;

    memory_arena * Arena;

    world_pos CurrentWorldP;

    entity           * ActiveEntities;
    //entity_transform * ActiveEntitiesTransform;
    u32                ActiveEntitiesCount;

    entity_input * EntitiesInput;


    entity * GroundEntities;
    entity ** HashGroundEntities;
    u32 HashGridGroundEntitiesSize;
    b32 * HashGroundOccupancy;
    u32 HashGridUsageCount;
    u32 GroundEntityLimit;
    u32 GroundEntityCount;

};

struct neighbor_iterator
{
    world_cell * Current;
    u32 CurrentNeighborIndex;
    cell_neighbor_offset * Neighbors;
    u32 CenterHashIndex;
    i32 x,y,z;
    b32 CanContinue;
};

struct simulation_mesh_obj_transform_iterator
{
    u32 Index;
    u32 MeshObjCount;
    entity_transform * T;
};

struct simulation_iterator
{
    entity * Entity;
    u32 Index;
    simulation * Sim;
    world * World;
};


GAME_API world
NewWorld(memory_arena * Arena, u32 DimX, u32 DimY, u32 DimZ);

GAME_API world_pos
WorldPosition(i32 X, i32 Y, i32 Z, v3 Offset = {0 , 0, 0});

GAME_API world_pos
MapIntoCell(world * World, world_pos P, v3 dP);

GAME_API entity *
AddEntity(world * World, world_pos P);

GAME_API neighbor_iterator
GetNeighborIterator(world * World,entity * Entity);

GAME_API void
AdvanceIterator(world * World, neighbor_iterator * Iterator);

GAME_API void
UpdateWorldLocation(world * World,simulation * Sim);

GAME_API v3
Substract(world * World, world_pos To, world_pos From);

/* SIMULATION */
GAME_API void
SimulationRegisterEntity(simulation * Sim, entity * Entity,  u32 StorageIndexID);

GAME_API void
BeginSimulation(world * World, simulation * Sim);

GAME_API simulation_iterator
BeginSimIterator(world * World, simulation * Sim);

GAME_API entity *
AdvanceSimIterator(simulation_iterator * Iterator);

GAME_API simulation_iterator
BeginSimGroundIterator(world * World, simulation * Sim);

GAME_API entity *
AdvanceSimGroundIterator(simulation_iterator * Iterator);


GAME_API simulation_mesh_obj_transform_iterator
BeginSimMeshObjTransformIterator(simulation * Sim, entity * Entity);

GAME_API entity_transform *  
AdvanceSimMeshObjTransformIterator(simulation_mesh_obj_transform_iterator * Iter);

GAME_API entity *
FindEntityByID(world * World,entity_id EntityID);

#endif
