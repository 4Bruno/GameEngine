#include "game.h"


sim_space *
CreateSimSpace(game_state * GameState, memory_arena * Arena)
{
    u32 TotalSizeEntities = GameState->PermanentArena.CurrentSize;
    GameState->LimitEntities = 4000;
    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity));
    GameState->Entities = (entity *)Base;
    GameState->TotalEntities = 0;

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(component_flags));
    GameState->EntitiesFlags = (component_flags *)Base;
    _FillArray(GameState->EntitiesFlags, GameState->LimitEntities, component_none);

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity));
    GameState->EntitiesParent = (entity *)Base;
    _FillArrayMember(GameState->EntitiesParent, GameState->LimitEntities, ID,NULL_ENTITY);

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity_transform));
    GameState->EntitiesTransform = (entity_transform *)Base;

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(collision));
    GameState->Collisions = (collision *)Base;

    GameState->LimitMeshes = 50;
    Base = PushSize(&GameState->PermanentArena, GameState->LimitMeshes * sizeof(mesh));
    GameState->Meshes = (mesh *)Base;

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(render_3D));
    GameState->Render3D = (render_3D *)Base;
    _FillArrayMember(GameState->Render3D,GameState->LimitEntities,MeshID,NULL_MESH);

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity_input));
    GameState->EntitiesInput = (entity_input *)Base;

    Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity_input));
    GameState->EntitiesMomentum = (entity_input *)Base;

    TotalSizeEntities = GameState->PermanentArena.CurrentSize - TotalSizeEntities;
    GameState->Player = NullEntity();

    GameState->TotalWorldEntitiesIDs = 0;

}
