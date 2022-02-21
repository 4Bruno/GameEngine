#ifndef GAME_ENTITY_H

#include "game_platform.h"
#include "game_math.h"
#include "game_world.h"
#include "game_mesh.h"

#define NULL_ENTITY 0
#define VALID_ENTITY(E) (E.ID > NULL_ENTITY)




struct async_update_entities_model
{
    u32 StartIndex;
    u32 EntitiesCount;
    game_state * GameState;
};

GAME_API
THREAD_WORK_HANDLER(AsyncUpdateEntitiesModel);

GAME_API void
UpdateGroundModel(world * World);

inline b32 
EntityHasFlag(entity * Entity, component_flags Flag)
{
    return (Entity->Flags & Flag);
}

GAME_API void
UpdateGroundEntity(entity * Entity, world_pos WorldP, world_pos ChunkP, v3 GroundScale);

GAME_API void
EntityAddTranslation(entity * Entity, entity * Parent, v3 P, v3 Scale, r32 Speed);

GAME_API void
UpdateTransform(entity * Entity);

GAME_API void
EntityAddMesh(entity * Entity, mesh_id MeshID, v3 Color = V3(1.0f,1.0f,1.0f), r32 Transparency = 0.0f);

GAME_API void
EntityDelete(entity * Entity);

inline entity
NullEntity()
{
    return { NULL_ENTITY };
}
#define GAME_ENTITY_H
#endif
