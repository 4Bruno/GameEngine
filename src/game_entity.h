#ifndef GAME_ENTITY_H

#include "game_platform.h"
#include "game_math.h"
#include "game_world.h"
#include "game_mesh.h"

#define NULL_ENTITY 0
#define VALID_ENTITY(E) (E.ID > NULL_ENTITY)


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

    mesh_id MeshID;
    u32 MeshObjCount;
    u32 MeshObjTransOcuppancyIndex;

    v3 Color; // 36

    // TODO: find different way to handle
    // with ground
    b32 IsGround;
    entity_transform Transform; // 170 + 36 = 206
};

struct entity_input
{
    v3 dP;
    v3 EulerXYZ;
};


struct async_update_entities_model
{
    u32 StartIndex;
    u32 EntitiesCount;
    game_state * GameState;
};

GAME_API
THREAD_WORK_HANDLER(AsyncUpdateEntitiesModel);

inline b32 
EntityHasFlag(entity * Entity, component_flags Flag)
{
    return (Entity->Flags & Flag);
}

GAME_API void
EntityAddTranslation(entity * Entity, entity * Parent, v3 P, v3 Scale, r32 Speed);

GAME_API void
EntityAddMesh(entity * Entity, mesh_id MeshID);

GAME_API void
EntityDelete(entity * Entity);

inline entity
NullEntity()
{
    return { NULL_ENTITY };
}
#define GAME_ENTITY_H
#endif
