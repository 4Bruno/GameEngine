#include "game.h"

inline entity
NullEntity()
{
    return { NULL_ENTITY };
}


inline entity_transform *
GetEntityTransform(game_state * GameState,entity Entity)
{
    entity_transform * T = &GameState->EntitiesTransform[Entity.ID];
    return T;
}

inline v3
GetEntityPos(game_state * GameState,entity Entity)
{
    v3 P = GameState->EntitiesTransform[Entity.ID].WorldP[3].xyz;
    
    return P;
}
