#ifndef GAME_ENTITY_H

#include "game_platform.h"
#include "game_math.h"
#include "Quaternion.h"
#include "game_world.h"
#include "game_mesh.h"

#define NULL_ENTITY UINT32_MAX
#define VALID_ENTITY(E) (E.ID != NULL_ENTITY)

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

    u32 MeshID;
    v3 Color; // 36

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

inline entity
NullEntity()
{
    return { NULL_ENTITY };
}
#define GAME_ENTITY_H
#endif
