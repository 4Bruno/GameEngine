#include "game.h"

entity
GetEntity(game_state * GameState,uint32 Index)
{
    entity Entity = GameState->Entities[Index];
    return Entity;
}

entity
NullEntity()
{
    return { NULL_ENTITY };
}


entity_transform *
GetEntityTransform(game_state * GameState,entity Entity)
{
    entity_transform * T = &GameState->EntitiesTransform[Entity.ID];
    return T;
}

v3
GetEntityPos(game_state * GameState,entity Entity)
{
    v3 P = GameState->EntitiesTransform[Entity.ID].WorldP[3].xyz;
    
    return P;
}

entity
AddEntity(game_state * GameState)
{
    Assert( 
            ((GameState->TotalEntities + 1) < GameState->LimitEntities) && 
            (GameState->LimitEntities > 0)
            );

    uint32 EntityID = GameState->TotalEntities++;

    entity Entity = { EntityID };

    GameState->Entities[EntityID] = Entity;

    return Entity;
}

void
EntityRemoveFlag(game_state * GameState, entity Entity, component_flags Flag)
{
    GameState->EntitiesFlags[Entity.ID] = (component_flags)(GameState->EntitiesFlags[Entity.ID] ^ Flag);
}

void
EntityAddFlag(game_state * GameState, entity Entity, component_flags Flag)
{
    GameState->EntitiesFlags[Entity.ID] = (component_flags)(GameState->EntitiesFlags[Entity.ID] | Flag);
}

bool32
EntityHasFlag(game_state * GameState,uint32 EntityIndex,component_flags Flag)
{
    uint32 EntityID = GameState->Entities[EntityIndex].ID;
    component_flags Flags = GameState->EntitiesFlags[EntityID];
    bool32 R = (Flags & Flag);

    return R;
}


v3
TestEntityMoveForward(entity_transform * T, v3 * dP)
{
    v3 TestP;
    Quaternion_rotate(&T->LocalR, dP, &TestP);
    TestP.x = -TestP.x;
    TestP += T->LocalP;

    return TestP;
}

void
RotateEntityFill(entity_transform * T, real32 AngleX, real32 AngleY, real32 AngleZ)
{
    v3 EulerXYZ = V3(AngleX, AngleY, AngleZ);
    Quaternion_fromEulerZYX(&EulerXYZ, &T->LocalR);
}

void
RotateEntityRight(entity_transform * T, real32 Angle)
{
    if (Angle == 0.0f) return;

    Quaternion Rotate;
    Quaternion_fromYRotation(Angle, &Rotate);
    Quaternion_multiply(&Rotate,&T->LocalR,&T->LocalR);
}

THREAD_WORK_HANDLER(AsyncUpdateEntitiesModel)
{
    async_update_entities_model * Update = (async_update_entities_model *)Data;
    game_state * GameState = Update->GameState;

    for (uint32 EntityIndex = Update->StartIndex;
                EntityIndex < (Update->StartIndex + Update->EntitiesCount);
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        uint32 EntityID = Entity.ID;
        if (EntityHasFlag(GameState,EntityID,component_transform))
        {
            entity Parent = GameState->EntitiesParent[EntityID]; 
            entity_transform * T = GameState->EntitiesTransform + EntityID;
            if (VALID_ENTITY(Parent))
            {
                uint32 ParentID = Parent.ID;
                entity_transform * ParentT = GameState->EntitiesTransform + ParentID;
                v3 LocalToWorldP;
                Quaternion_rotate(&ParentT->WorldR,&T->LocalP,&LocalToWorldP);
                Translate(T->WorldP,(ParentT->LocalP + LocalToWorldP));
                Quaternion_multiply(&ParentT->WorldR,&T->LocalR,&ParentT->WorldR);
                T->WorldS = { 
                    ParentT->WorldS.x * T->LocalS.x,
                    ParentT->WorldS.y * T->LocalS.y,
                    ParentT->WorldS.z * T->LocalS.z
                };
            }
            else
            {
                Translate(T->WorldP,T->LocalP);
                T->WorldR = T->LocalR;
                T->WorldS = T->LocalS;
            }
            m4 R = Quaternion_toMatrix(T->WorldR);
            R[0].x = -R[0].x;
            R[1].x = -R[1].x;
            R[2].x = -R[2].x;
            //Log("Pitch: %f, Yaw: %f\n",T->Pitch,T->Yaw);
            T->WorldT = T->WorldP * R * M4(T->WorldS);
            //Log("Entity: %i",EntityIndex);LOG_P(GetMatrixPos(T->WorldP));
        }
    }
}

void
UpdateEntitiesModel(game_state * GameState)
{
    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        uint32 EntityID = Entity.ID;
        if (EntityHasFlag(GameState,EntityID,component_transform))
        {
            entity Parent = GameState->EntitiesParent[EntityID]; 
            entity_transform * T = GameState->EntitiesTransform + EntityID;
            if (VALID_ENTITY(Parent))
            {
                uint32 ParentID = Parent.ID;
                entity_transform * ParentT = GameState->EntitiesTransform + ParentID;
                v3 LocalToWorldP;
                Quaternion_rotate(&ParentT->WorldR,&T->LocalP,&LocalToWorldP);
                Translate(T->WorldP,(ParentT->LocalP + LocalToWorldP));
                Quaternion_multiply(&ParentT->WorldR,&T->LocalR,&ParentT->WorldR);
                T->WorldS = { 
                    ParentT->WorldS.x * T->LocalS.x,
                    ParentT->WorldS.y * T->LocalS.y,
                    ParentT->WorldS.z * T->LocalS.z
                };
            }
            else
            {
                Translate(T->WorldP,T->LocalP);
                T->WorldR = T->LocalR;
                T->WorldS = T->LocalS;
            }
            m4 R = Quaternion_toMatrix(T->WorldR);
            R[0].x = -R[0].x;
            R[1].x = -R[1].x;
            R[2].x = -R[2].x;
            T->WorldT = T->WorldP * R * M4(T->WorldS);
        }
    }
}

void
EntityAddTranslation(game_state * GameState, entity Entity, entity Parent, v3 P, v3 Scale, real32 Speed)
{
    entity_transform * T = (GameState->EntitiesTransform + Entity.ID);
    T->LocalP = P;
    T->LocalS = Scale;
    Quaternion_setIdentity(&T->LocalR);
    Translate(T->WorldP,T->LocalP);
    T->WorldS = Scale;
    T->WorldR = T->LocalR;
    T->WorldT = {};

    GameState->EntitiesParent[Entity.ID] = Parent;

    EntityAddFlag(GameState,Entity,component_transform);
}
