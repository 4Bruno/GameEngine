#ifndef GAME_WORLD_H
#define GAME_WORLD_H
/*
 * https://www10.cs.fau.de/publications/theses/2009/Schornbaum_SA_2009.pdf
 */

#include "game_platform.h"
#include "game_memory.h"
#include "game_math.h"

#define MAX_WORLD_ENTITY_COUNT (1 << 14)
#define MAX_WORLD_ENTITY_COUNT_MINUS_ONE (MAX_WORLD_ENTITY_COUNT - 1)

// Helpers to printf world positions
#define STRP "fx:%f fy:%f fz:%f"
#define STRWORLDP "X:%i Y:%i Z:%i " STRP
#define FWORLDP(P) P.x, P.y, P.z, P._Offset.x, P._Offset.y, P._Offset.z
#define FP(P) P.x, P.y, P.z

// game_entity.h
struct entity_transform;
struct entity;
struct entity_input;

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


struct simulation
{
    world_pos Origin;    
    v3 Dim;
    
    // sparse array to world entities access
    sparse_entry EntityEntries[MAX_WORLD_ENTITY_COUNT];
    u32 EntryCount;

    sparse_entry TransformEntries[MAX_WORLD_ENTITY_COUNT];
    u32 TransformCount;

    u32 SortingBuckets[MAX_WORLD_ENTITY_COUNT];
    
};



/*
 * Large buffer to accommodate a big number of entities data
 * if cell needs to add more data and is not sufficient
 * it uses linked list to add new chunk of data
 */
#define WORLD_CELL_DATA_SIZE (1 << 16)
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

#endif