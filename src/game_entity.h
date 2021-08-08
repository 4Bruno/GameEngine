#ifndef GAME_ENTITY_H

#include "game_platform.h"
#include "game_math.h"
#include "Quaternion.h"

#define NULL_ENTITY UINT32_MAX
#define VALID_ENTITY(E) (E.ID != NULL_ENTITY)

enum component_flags
{
    component_none = 0 << 0,

    component_input     = 1 << 0,
    component_collision = 1 << 1,
    component_render_3d = 1 << 2,
    component_transform = 1 << 3,
    component_momentum  = 1 << 4,
    component_ground    = 1 << 5

};

struct entity
{
    uint32 ID;
};

struct entity_transform
{
    v3 LocalP;
    v3 LocalS;
    Quaternion LocalR;

    real32 Yaw, Pitch;

    m4 WorldP;
    v3 WorldS;
    Quaternion WorldR;

    m4 WorldT;
};

struct async_update_entities_model
{
    uint32 StartIndex;
    uint32 EntitiesCount;
    game_state * GameState;
};

entity
NullEntity();

entity_transform *
GetEntityTransform(game_state * GameState,entity Entity);

v3
GetEntityPos(game_state * GameState,entity Entity);

entity
GetEntity(game_state * GameState,uint32 Index);

entity
AddEntity(game_state * GameState);

void
EntityAddTranslation(game_state * GameState, entity Entity, entity Parent, v3 P, v3 Scale, real32 Speed);

void
EntityRemoveFlag(game_state * GameState, entity Entity, component_flags Flag);

void
EntityAddFlag(game_state * GameState, entity Entity, component_flags Flag);

bool32
EntityHasFlag(game_state * GameState,uint32 EntityIndex,component_flags Flag);

v3
TestEntityMoveForward(entity_transform * T, v3 * dP);

void
RotateEntityRight(entity_transform * T, real32 Angle);

void
RotateEntityFill(entity_transform * T, real32 AngleX, real32 AngleY, real32 AngleZ);

THREAD_WORK_HANDLER(AsyncUpdateEntitiesModel);

#define GAME_ENTITY_H
#endif
