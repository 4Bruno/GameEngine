#include "game.h"
#include "math.h"
#include "model_loader.cpp"
#include "render_2.cpp"
#include "collision.cpp"


#define LOG_P(P) Log("x: %f y: %f z: %f \n", P.x, P.y, P.z);

enum  shader_type_vertex
{
    shader_type_vertex_default,


    shader_type_vertex_total
};

enum  shader_type_fragment
{
    shader_type_fragment_default,


    shader_type_fragment_total
};

struct file_contents
{
    uint8 * Base;
    uint32 Size;
    bool32 Success;
};

file_contents
GetFileContents(game_memory * GameMemory,memory_arena * Arena,const char * Filepath)
{
    file_contents Result = {};
    void * Buffer = 0;

    platform_open_file_result OpenFileResult = GameMemory->DebugOpenFile(Filepath);

    if (OpenFileResult.Success)
    {
        Buffer = PushSize(Arena,OpenFileResult.Size);
        if (GameMemory->DebugReadFile(OpenFileResult,Buffer))
        {
            Result.Size = OpenFileResult.Size;
            Result.Base = (uint8 *)Buffer;
            Result.Success = true;
        }
        else
        {
            Arena->CurrentSize -= OpenFileResult.Size;
        }
        GameMemory->DebugCloseFile(OpenFileResult);
    }
    return Result;
}

void
InitializeArena(memory_arena * Arena,uint8 * BaseAddr, uint32 MaxSize)
{
    Arena->MaxSize = MaxSize;
    Arena->CurrentSize = 0;
    Arena->Base = BaseAddr;
}

mesh
LoadModel(game_memory * Memory,memory_arena * ArenaMeshes, memory_arena * ArenaTemp,const char * Filepath)
{
    mesh Mesh = {};
    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(Memory, ArenaTemp,Filepath);
    if (GetFileResult.Success)
    {
        obj_file_header Header = ReadObjFileHeader((const char *)GetFileResult.Base, GetFileResult.Size);
        Mesh = CreateMeshFromObjHeader(ArenaMeshes,Header, (const char *)GetFileResult.Base, GetFileResult.Size);
        ArenaTemp->CurrentSize -= GetFileResult.Size;
    }

    return Mesh;
}

int32
LoadShader(game_memory * Memory,memory_arena * Arena,const char * Filepath)
{
    int32 Result = -1;
    file_contents GetFileResult = GetFileContents(Memory, Arena,Filepath);
    if (GetFileResult.Success)
    {
        Result = RenderCreateShaderModule((char *)GetFileResult.Base, (size_t)GetFileResult.Size);
        // data lives on gpu side now
        Arena->CurrentSize -= GetFileResult.Size;
    }
    return Result;
}
#if 0
mesh
CreateTriangle(memory_arena * Arena, v3 P, real32 SideLength)
{
    mesh Mesh = {};

    uint32 Vertices = 3;
    Mesh.VertexSize  = sizeof(vertex_point)*Vertices;
    Mesh.Vertices    = PushArray(Arena, 3, vertex_point); // vertex_point * Vertices;
    Mesh.Vertices[0].P = P;
    Mesh.Vertices[1].P = (P + V3(SideLength, SideLength,0));
    Mesh.Vertices[2].P = (P + V3(SideLength*2.0f, 0,0));

    return Mesh;
}
#endif

void
MoveEntity(entity_input * Input, v3 D, real32 dtTime, real32 Yaw, real32 Pitch, real32 Gravity)
{
#if 0
    // TODO: where should speed go?
    real32 Speed = 3.0f * dtTime;

    D.y = -1.0f; // gravity

    if (LengthSqr(D) > 1.0f)
    {
        D = Normalize(D);
    }

    Entity->dP = V3(D.x * Speed, D.y * Gravity, D.z * Gravity);

#if 0
    if (Yaw)
    {
        Entity->Yaw += Yaw * 10.0f;
    }
    if (Pitch)
    {
        Entity->Pitch += Pitch * 10.0f;
    }
    if (Entity->Pitch > 89.9f)
    {
        Entity->Pitch = 89.9f;
    }
    else if (Entity->Pitch < -89.9f)
    {
        Entity->Pitch = -89.9f;
    }
#endif
#else
    // Implement
#endif
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

void
EntityAddTranslation(game_state * GameState, entity Entity, entity Parent, v3 P, v3 Scale, real32 Speed)
{
    entity_transform * T = (GameState->EntitiesTransform + Entity.ID);
    Translate(T->LocalP,P);
    T->LocalS = Scale;
    T->LocalR = M4();
    T->WorldP = {};
    T->WorldS = {};
    T->WorldR = {};
    T->WorldT = {};

    GameState->EntitiesParent[Entity.ID] = Parent;

    EntityAddFlag(GameState,Entity,component_transform);
}

void
EntityAddCollision(game_state * GameState, entity Entity)
{
#if 0
    Assert((GameState->TotalCollisionSpheres + 1) < ArrayCount(GameState->CollisionSpheres));
    RitterSphere(GameState->CollisionSpheres, Entity->Mesh->Vertices,Entity->Mesh->VertexSize);
#endif
    EntityAddFlag(GameState,Entity,component_collision);
}

void
EntityAddInput(game_state * GameState,entity Entity, v3 dP)
{
    GameState->EntitiesInput[Entity.ID].dP = dP;
    EntityAddFlag(GameState,Entity,component_input);
}


void
EntityAdd3DRender(game_state * GameState, entity Entity, uint32 MeshID)
{
    Assert(VALID_MESH_ID(MeshID));
    (GameState->Render3D + Entity.ID)->MeshID = MeshID;
    GameState->EntitiesFlags[Entity.ID] = (component_flags)(GameState->EntitiesFlags[Entity.ID] | component_render_3d);
}

void
FreeCameraView(game_state * GameState, v3 dP, real32 Yaw, real32 Pitch)
{
    if (dP.z)
    {
        MoveViewForward(GameState, -dP.z);
    }
    if (dP.x)
    {
        MoveViewRight(GameState, dP.x);
    }
    if (Yaw)
    {
        GameState->Camera.Yaw += -Yaw * 10.0f;
    }
    if (Pitch)
    {
        GameState->Camera.Pitch += Pitch * 10.0f;
    }
    if (GameState->Camera.Pitch > 89.9f)
    {
        GameState->Camera.Pitch = 89.9f;
    }
    else if (GameState->Camera.Pitch < -89.9f)
    {
        GameState->Camera.Pitch = -89.9f;
    }
    GameState->ViewRotationMatrix = M4();
    RotateFill(&GameState->ViewRotationMatrix, ToRadians(GameState->Camera.Pitch), ToRadians(GameState->Camera.Yaw), 0);
    UpdateView(GameState);
}

bool32
FirstTimePressed(game_button * Button)
{
    bool32 Result = (Button->IsPressed && !Button->WasPressed);
    return Result;
}

SCENE_HANDLER(HandleSceneFloor)
{

    entity * SourceLight = Scene->LightSources[0];

#if 0
    if (Input->Controller.Numbers[1].IsPressed)
    {
        GameState->CameraMode = true;
    }
    else if (Input->Controller.Numbers[2].IsPressed)
    {
        GameState->CameraMode = false;
    }
    if (GameState->CameraMode)
    {
        FreeCameraView(GameState, dP, Yaw,Pitch);
    }
    else
    {
        //entity * Entity = GameState->Player;
        entity * Entity = SourceLight;
        MoveEntity(Entity, dP,Input->DtFrame, Yaw,Pitch, 9.8f);
        SetViewBehindObject(&GameState->Renderer,Entity->P,Entity->D,5.0f, 2.0f);
    }
#endif

}

SCENE_LOADER(LoadSceneFloor)
{

    entity Entity = AddEntity(GameState);
    EntityAddTranslation(GameState,Entity,NullEntity(),V3(0,-2.0f,0),V3(8.0f,1.0f,8.0f),3.0f);
    EntityAdd3DRender(GameState,Entity,0);

    Entity = AddEntity(GameState);
    EntityAddTranslation(GameState,Entity,NullEntity(),V3(0,0,0),V3(1,1,1),3.0f);
    EntityAdd3DRender(GameState,Entity,0);

    GameState->Player = Entity;

    ViewLookAt(GameState,V3(0,0.0f,5.0f),V3(0,0,0));

    GameState->Camera.D = GetViewDirection(GameState->ViewRotationMatrix);
    GameState->Camera.Yaw = 0.0f;
    GameState->Camera.Pitch = 0.0f;
}


void
CreatePipeline(game_memory * Memory,game_state * GameState)
{
    GameState->ShadersArena.CurrentSize = 0;
    RenderFreeShaders();
    VulkanDestroyPipeline();

    GameState->VertexShaders[shader_type_vertex_default] 
        = LoadShader(Memory,&GameState->ShadersArena,"shaders\\triangle.vert");
    GameState->FragmentShaders[shader_type_fragment_default] 
        = LoadShader(Memory,&GameState->ShadersArena,"shaders\\triangle.frag");

    GameState->PipelineIndex = 
        RenderCreatePipeline(GameState->VertexShaders[shader_type_vertex_default], 
                GameState->FragmentShaders[shader_type_fragment_default]);

    // TODO: destroy shaders after pipeline creation

    if (GameState->PipelineIndex < 0)
    {
        Log("Error during creation of main pipeline\n");
    }

    Assert(GameState->PipelineIndex >= 0);
}

inline bool32
EntityHasFlag(game_state * GameState,uint32 EntityIndex,component_flags Flag)
{
    uint32 EntityID = GameState->Entities[EntityIndex].ID;
    component_flags Flags = GameState->EntitiesFlags[EntityID];
    bool32 R = (Flags & Flag);

    return R;
}

void
LoadMesh(game_memory * Memory,game_state * GameState, const char * Path)
{
    Assert(GameState->LimitMeshes > (GameState->TotalMeshes + 1));
    GameState->Meshes[GameState->TotalMeshes] = LoadModel(Memory,&GameState->MeshesArena,&GameState->TemporaryArena,Path);
    ++GameState->TotalMeshes;
}

#define _FillArray(A,C,V) \
    for (uint32 i = 0; \
            i < C; \
            ++i) \
    { \
        A[i] = V; \
    }

#define _FillArrayMember(A,C,M,V) \
    for (uint32 i = 0; \
            i < C; \
            ++i) \
    { \
        A[i].M = V; \
    }


extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state * GameState = (game_state *)Memory->PermanentMemory;

    if (!GameState->IsInitialized)
    {
        /* ------------------------ Fixed number of entities arrays ----------------------------- */
        uint8 * Base = ((uint8 *)Memory->PermanentMemory + sizeof(game_state));
        int32 AvailablePermanentMemory = Memory->PermanentMemorySize - sizeof(game_state);
        InitializeArena(&GameState->PermanentArena,Base, AvailablePermanentMemory);

        GameState->LimitEntities = 100;
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
        GameState->TotalMeshes = 0;

        Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(render_3D));
        GameState->Render3D = (render_3D *)Base;
        _FillArrayMember(GameState->Render3D,GameState->LimitEntities,MeshID,NULL_MESH);

        Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity_input));
        GameState->EntitiesInput = (entity_input *)Base;

        Base = PushSize(&GameState->PermanentArena, GameState->LimitEntities * sizeof(entity_input));
        GameState->EntitiesMomentum = (entity_input *)Base;

        GameState->Player = NullEntity();

        /* ------------------------ Temporary arenas ----------------------------- */
        Base = (uint8 *)Memory->TransientMemory;
        InitializeArena(&GameState->TemporaryArena,Base, Memory->TransientMemorySize);

        uint32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        uint32 MeshesArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);

        LoadMesh(Memory,GameState,"assets\\cube.obj");

        // TODO: how to properly push vertex inputs to render?
        // Game should tell how much arena?
        // Should buffers be game runtime?
        // or we always reserve a size and then use like we do with cpu memory
        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        for (uint32 MeshIndex = 0;
                MeshIndex < GameState->TotalMeshes;
                ++MeshIndex)
        {
            mesh * Mesh = GameState->Meshes + MeshIndex; 

            Mesh->OffsetVertices = GameState->VertexArena.CurrentSize;
            //Mesh->OffsetIndices = GameState->IndicesArena.CurrentSize;

            RenderPushVertexData(&GameState->VertexArena, Mesh->Vertices,Mesh->VertexSize, 1);
            //RenderPushIndexData(&GameState->IndicesArena, Mesh->Indices,Mesh->IndicesSize, 1);
        }

        CreatePipeline(Memory,GameState);

        v3 WorldCenter = V3(0,0,0);

        WorldInitializeView(GameState, (ToRadians(70.0f)), 
                           ScreenWidth,ScreenHeight, 
                           0.1f, 200.0f, 
                           WorldCenter);

        LoadSceneFloor(GameState);

        GameState->CameraMode = true;

        GameState->IsInitialized = true;
    }

    if (Input->Reloaded)
    {
        // Game DLL reloaded
        //LoadSceneFloor(GameState);
    }

    if (Input->ShaderHasChanged)
    {
        CreatePipeline(Memory,GameState);
        Input->ShaderHasChanged = false;
    }

    real32 MouseX = (real32)Input->Controller.RelMouseX;
    real32 MouseY = (real32)Input->Controller.RelMouseY;
    
    //if (MouseX != 0.0f || MouseY != 0.0f) Log("x: %f y:%f\n",MouseX,MouseY);

    v3 MouseRotation = V3(MouseX,MouseY,0) / Length(V3(MouseX,MouseY,0));

    if (Input->Controller.R.IsPressed)
    {
        GameState->TotalEntities = 0;
        LoadSceneFloor(GameState);
    }

    /* ------------------------- GAME UPDATE ------------------------- */

    {
        v3 dP = {};

        if (Input->Controller.Up.IsPressed)
        {
            dP.z = 1.0f;
        } 
        if (Input->Controller.Down.IsPressed)
        {
            dP.z = -1.0f;
        }
        if (Input->Controller.Left.IsPressed)
        {
            dP.x = -1.0f;
        } 
        if (Input->Controller.Right.IsPressed)
        {
            dP.x = 1.0f;
        }
        if (Input->Controller.Space.IsPressed)
        {
            dP.z = 1.0f;
        }

        real32 Yaw = (MouseX != 0.0f) ? (MouseX / 20.0f) : 0.0f;
        real32 Pitch= (MouseY != 0.0f) ? (MouseY / 20.0f)   : 0.0f;

        if ((dP.x != 0.0f || dP.y != 0.0f || dP.z != 0.0f) && VALID_ENTITY(GameState->Player))
        {
            EntityAddInput(GameState,GameState->Player,dP);
        }
    }

    /* ----------------------- GAME UPDATE ---------------------------- */
    // TODO: where should speed go?
    real32 Speed = 3.0f * Input->DtFrame;
    real32 Gravity = 0.0f;//9.8f;

    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        uint32 EntityID = GameState->Entities[EntityIndex].ID;

        if (EntityHasFlag(GameState,EntityID,component_input))
        {
            v3 dP = GameState->EntitiesInput[EntityID].dP;

            dP.y = -1.0f; // gravity

            if (LengthSqr(dP) > 1.0f)
            {
                dP = Normalize(dP);
            }

            GameState->EntitiesMomentum[EntityID].dP = V3(dP.x * Speed, dP.y * Gravity, dP.z * Speed);
            EntityRemoveFlag(GameState,GameState->Entities[EntityIndex],component_input);
            EntityAddFlag(GameState,GameState->Entities[EntityIndex],component_momentum);
        }
    }

    // collision
    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        entity Entity = GameState->Entities[EntityIndex];
        uint32 EntityID = Entity.ID;

        if (EntityHasFlag(GameState,EntityID,component_momentum))
        {
            v3 dP = GameState->EntitiesMomentum[EntityID].dP;
            entity_transform * ET = GameState->EntitiesTransform + EntityID;
            v3 EntityP = GetEntityPos(ET->WorldP);
            v3 MoveFw = GetEntityDirection(ET->WorldR) * dP.z;
            v3 MoveRight = GetEntityRight(ET->WorldR) * dP.x;
            v3 DesiredP = EntityP + MoveFw + MoveRight + V3(0,-1,0) * dP.y;

            v3 MaxMove = { 1.0f, 1.0f, 1.0f };

            if (EntityHasFlag(GameState,EntityID,component_collision))
            {
#if 0

                // TODO: handle collision
                for (uint32 EntityTestIndex = 0;
                        EntityTestIndex < Scene->EntityCount;
                        ++EntityTestIndex)
                {
                    entity * EntityTest = Scene->Entities[EntityIndex];
                    if (EntityTest->CollisionSphere)
                    {
                        sphere EntityTestSphere = *EntityTest->CollisionSphere;
                        if (TestSphereSphere(EntitySphere, EntityTestSphere))
                        {
                            // % movement allowed
                            MaxMove = CheckCollision(Entity,EntityTest);
                        }
                    }
                }
#endif
            } // entity has collision

            EntityP = VectorMultiply(DesiredP,MaxMove);

            Translate(ET->LocalP, EntityP);

            // TODO: explore other non-dampening techniques
            GameState->EntitiesMomentum[EntityID].dP *= 0.85f;//dP - dP*0.5f;

            dP = GameState->EntitiesMomentum[EntityID].dP;

            if (LengthSqr(dP) < 0.001f)
                EntityRemoveFlag(GameState,Entity,component_momentum);

        } // has momentum
    }

    /* ----------------------- Model Transform ------------------------------- */

    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        uint32 EntityID = GameState->Entities[EntityIndex].ID;
        if (EntityHasFlag(GameState,EntityID,component_transform))
        {
            entity Parent = GameState->EntitiesParent[EntityID]; 
            entity_transform * T = GameState->EntitiesTransform + EntityID;
            if (VALID_ENTITY(Parent))
            {
                uint32 ParentID = Parent.ID;
                entity_transform * ParentT = GameState->EntitiesTransform + ParentID;
                T->WorldP = ParentT->WorldP + (ParentT->WorldR * T->LocalP);
                T->WorldR = ParentT->WorldR * T->LocalR;
                T->WorldS = { 
                    ParentT->WorldS.x * T->LocalS.x,
                    ParentT->WorldS.y * T->LocalS.y,
                    ParentT->WorldS.z * T->LocalS.z
                };
            }
            else
            {
                T->WorldP = T->LocalP;
                T->WorldR = T->LocalR;
                T->WorldS = T->LocalS;
            }
            T->WorldT = T->WorldP * T->WorldR * M4(T->WorldS);
        }
    }

    RotateRight(&GameState->ViewRotationMatrix, ((sinf(Input->TimeElapsed) + 1.0f) * 0.5f)*PI*0.01f);

    /* ------------------------- GAME RENDER ------------------------- */

    v4 ClearColor = V4(0,0,0,1);

    BeginRender(GameState, ClearColor);

    RenderEntities(GameState);
    
    EndRender(GameState);
}

