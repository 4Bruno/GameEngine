#ifndef GAME_SPATIAL_H

#include "game_math.h"
#include "game_world.h"
#include "game_entity.h"


// this is just for safety
struct entity_ptr
{
    entity * Ptr;
};

/*
 * Simulation space is a rectangle covering world cells
 * 
 */
struct sim_space
{
    world_pos Origin;
    v3 Dim;

    entity * Entities;
    u32 LimitEntities;
    u32 TotalEntities;

    component_flags * EntitiesFlags;

    entity * EntitiesParent;

    // Component position/rotation/scale
    entity_transform * EntitiesTransform;

    // Component Input
    entity_input * EntitiesInput;
    entity_input * EntitiesMomentum;

    // Component collision
    collision * Collisions;

    // Component rendering
    render_3D * Render3D;
};

#define GAME_SPATIAL_H
#endif


