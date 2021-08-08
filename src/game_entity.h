#ifndef GAME_ENTITY_H

#include "game_platform.h"
#include "game_math.h"

#define NULL_ENTITY UINT32_MAX
#define VALID_ENTITY(E) (E.ID != NULL_ENTITY)

struct entity
{
    uint32 ID;
};

struct entity_transform
{
    m4 LocalP;
    v3 LocalS;
    m4 LocalR;    
    real32 Yaw, Pitch;

    m4 WorldP;
    v3 WorldS;
    m4 WorldR;    

    m4 WorldT;
};

struct world
{
    entity * EntitiesParent;

    // Component position/rotation/scale
    entity_transform * EntitiesTransform;

};

GAME_API inline entity
NullEntity();

GAME_API inline entity_transform *
GetEntityTransform(game_state * GameState,entity Entity);

GAME_API inline v3
GetEntityPos(game_state * GameState,entity Entity);

#define GAME_ENTITY_H
#endif
