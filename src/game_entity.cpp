#include "game.h"


#if 0
entity
GetEntity(game_state * GameState,u32 Index)
{
    entity Entity = GameState->Entities[Index];
    return Entity;
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
RotateEntityFill(entity_transform * T, r32 AngleX, r32 AngleY, r32 AngleZ)
{
    v3 EulerXYZ = V3(AngleX, AngleY, AngleZ);
    Quaternion_fromEulerZYX(&EulerXYZ, &T->LocalR);
}

void
RotateEntityRight(entity_transform * T, r32 Angle)
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

    for (u32 EntityIndex = Update->StartIndex;
                EntityIndex < (Update->StartIndex + Update->EntitiesCount);
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        u32 EntityID = Entity.ID;
        if (EntityHasFlag(GameState,EntityID,component_transform))
        {
            entity Parent = GameState->EntitiesParent[EntityID]; 
            entity_transform * T = GameState->EntitiesTransform + EntityID;
            if (VALID_ENTITY(Parent))
            {
                u32 ParentID = Parent.ID;
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
    for (u32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        u32 EntityID = Entity.ID;
        if (EntityHasFlag(GameState,EntityID,component_transform))
        {
            entity Parent = GameState->EntitiesParent[EntityID]; 
            entity_transform * T = GameState->EntitiesTransform + EntityID;
            if (VALID_ENTITY(Parent))
            {
                u32 ParentID = Parent.ID;
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

#endif

THREAD_WORK_HANDLER(AsyncUpdateEntitiesModel)
{
    async_update_entities_model * Update = (async_update_entities_model *)Data;
    game_state * GameState = Update->GameState;

    for (u32 EntityIndex = Update->StartIndex;
                EntityIndex < (Update->StartIndex + Update->EntitiesCount);
                ++EntityIndex)
    {
        entity * Entity = GameState->World.ActiveEntities + EntityIndex;
        if (EntityHasFlag(Entity,component_transform))
        {
            entity_transform * T = &Entity->Transform;

            Translate(T->WorldP,T->LocalP);
            T->WorldR = T->LocalR;
            T->WorldS = T->LocalS;
            m4 R = Quaternion_toMatrix(T->WorldR);
            R[0].x = -R[0].x;
            R[1].x = -R[1].x;
            R[2].x = -R[2].x;
            //Log("Pitch: %f, Yaw: %f\n",T->Pitch,T->Yaw);
            T->WorldT = T->WorldP * R * M4(T->WorldS);
            Log("Entity: %i",EntityIndex);LOG_P(GetMatrixPos(T->WorldP));
            
            simulation * Sim = GameState->Simulation;
            if ( (Entity->MeshObjCount > 1) && IS_NOT_NULL(Sim) )
            {
                simulation_mesh_obj_transform_iterator Iterator =
                    BeginSimMeshObjTransformIterator(Sim, Entity);

                for (entity_transform * MeshObjT = Iterator.T;
                        IS_NOT_NULL(MeshObjT);
                        MeshObjT = AdvanceSimMeshObjTransformIterator(&Iterator))
                {
                    v3 LocalToWorldP;
                    Quaternion_rotate(&T->WorldR,&MeshObjT->LocalP,&LocalToWorldP);
                    Translate(MeshObjT->WorldP,(T->LocalP + LocalToWorldP));
                    Quaternion_multiply(&T->WorldR,&MeshObjT->LocalR,&MeshObjT->WorldR);
                    MeshObjT->WorldS = { 
                        T->WorldS.x * MeshObjT->LocalS.x,
                        T->WorldS.y * MeshObjT->LocalS.y,
                        T->WorldS.z * MeshObjT->LocalS.z
                    };
                    R = Quaternion_toMatrix(MeshObjT->WorldR);
                    R[0].x = -R[0].x;
                    R[1].x = -R[1].x;
                    R[2].x = -R[2].x;
                    //Log("Pitch: %f, Yaw: %f\n",MeshObjT->Pitch,MeshObjT->Yaw);
                    MeshObjT->WorldT = MeshObjT->WorldP * R * M4(MeshObjT->WorldS);
                    //Log("Entity: %i Object: %i",Entity->ID.ID,Iterator.Index);
                    //LOG_P(GetMatrixPos(MeshObjT->WorldP));
                }
            }
        }
    }
}

void
EntityAddFlag(entity * Entity, component_flags Flag)
{
    Entity->Flags = (component_flags)(Entity->Flags | Flag);
}

void
EntityAddTranslation(entity * Entity, entity * Parent, v3 P, v3 Scale, r32 Speed)
{
    
    entity_transform * T = &Entity->Transform;
    T->LocalP = P;
    T->LocalS = Scale;
    Quaternion_setIdentity(&T->LocalR);
    Translate(T->WorldP,T->LocalP);
    T->WorldS = Scale;
    T->WorldR = T->LocalR;
    T->WorldT = {};

    EntityAddFlag(Entity,component_transform);
}

void
EntityDelete(entity * Entity)
{
    EntityAddFlag(Entity,component_delete);
}

void
EntityAddMesh(entity * Entity, mesh_id MeshID)
{
    mesh_group Mesh = GetMeshInfo(MeshID);
    Entity->MeshID = MeshID;
    Entity->MeshObjCount = Mesh.TotalMeshObjects;
#if 0
    mesh_group Mesh = GestMeshInfo(MeshID.ID);
    for (uint32 MeshIndex = 0;
                MeshIndex < Mesh.TotalMeshObjects;
                ++MeshIndex)
    {
        entity * MeshObjEntity = AddEntity(GameState->World,Entity->WorldP);
        MeshObjEntity->Parent = Entity;
    }
    
    Mesh.TotalMeshObjects
#endif
}
