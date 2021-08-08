#include "game.h"

#define LOG_P(P) Log("x: %f y: %f z: %f \n", P.x, P.y, P.z);




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


v3
UniformScaleFromHeight(real32 LocalModelHeightInUnits, real32 DesiredHeightInMeters)
{
    real32 MetersToUnits = 0.5f; // 2 units == 1 metter 
    real32 ScaleFactor = DesiredHeightInMeters / (LocalModelHeightInUnits * MetersToUnits);
    v3 S = V3(ScaleFactor);

    return S;
}

void
EntityAddInput(game_state * GameState,entity Entity, v3 dP)
{
    GameState->EntitiesInput[Entity.ID].dP = dP;
    EntityAddFlag(GameState,Entity,component_input);
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
}

bool32
FirstTimePressed(game_button * Button)
{
    bool32 Result = (Button->IsPressed && !Button->WasPressed);
    return Result;
}

struct scene_data
{
    uint32 CurrentNode;
    uint32 MaxNode;
    v3 * Max;
    v3 * Min;
    vertex_point * Vertices;
};

SCENE_HANDLER(HandleDebugConvexHull)
{
    if (FirstTimePressed(&Input->Controller.Numbers[1]))
    {
        real32 Scale = 0.1f;
        scene_data * Data = (scene_data *)GameState->SceneData;
        if (Data->CurrentNode < Data->MaxNode)
        {
            if (!Data->Max)
            {
                // Find max/min
            }
            else
            {
                v3 P = Data->Vertices[Data->CurrentNode].P;
                v3 ViewP = GetViewPos(GameState);
                P.z = ViewP.z - 10.0f;
                P.x = ViewP.x;
                P.y = 50.0f;
                entity Entity = AddEntity(GameState);
                EntityAddTranslation(GameState,
                        Entity,NullEntity(),
                        P,
                        V3(Scale,Scale,Scale),1.0f);
                EntityAdd3DRender(GameState,Entity,0,RGB_RED);

                //LOG_P(P);
                ++Data->CurrentNode;
            }
        }
    }

}

SCENE_LOADER(LoadSceneDebugConvexHull)
{

    ViewLookAt(GameState,V3(0,0.0f,5.0f),V3(0,0,0));

    GameState->Camera.D = GetMatrixDirection(GameState->ViewRotationMatrix);
    GameState->Camera.Yaw = 0.0f;
    GameState->Camera.Pitch = 0.0f;

    GameState->SceneData = PushSize(&GameState->TemporaryArena, sizeof(scene_data));
    scene_data * Data = (scene_data *)GameState->SceneData;
    Data->Vertices = GameState->Meshes[0].Vertices;
    Data->CurrentNode = 0;
    Data->MaxNode = GameState->Meshes[0].VertexSize / sizeof(vertex_point);
}

SCENE_LOADER(LoadSceneFloor)
{
    WorldInitializeView(GameState, (ToRadians(90.0f)), ScreenX,ScreenY, 0.1f, 200.0f, V3(0,0,0));

    real32 DepthZ = 5.0f;
    v3 P = V3(0, 0, -DepthZ);
    v3 CameraP = P + V3(0,0,5.0f);

    real32 CubeWidth = 0.1139f;
    real32 CubeHeight = 0.115323f;
    //v3 S = V3(CubeWidth,CubeHeight,1.0f) * 1.1f;
    v3 S = V3(1.0f) * 0.113927f;
    entity Entity = AddEntity(GameState);
    //EntityAddTranslation(GameState,Entity,NullEntity(),V3(0,0,0),V3(0.1f,0.1f,0.3f),3.0f);
    EntityAddTranslation(GameState,Entity,NullEntity(),P,S,3.0f);
    EntityAdd3DRender(GameState,Entity,1,RGB_GREY);
    //EntityLookAt(GameState,Entity,V3(0,0,-10.0f));
    GameState->Player = Entity;

    real32 WidthOverHeight = (real32)ScreenX / (real32)ScreenY;
    real32 FOV = 45.0f;
    real32 TanFOV = tanf(ToRadians(FOV));
    real32 EntityToCameraZ = fabsf((P - CameraP).z);
    real32 Height = (TanFOV * EntityToCameraZ) * 2.0f;
    real32 Width = (TanFOV * WidthOverHeight * EntityToCameraZ) * 2.0f;

    Entity = AddEntity(GameState);
    v3 OffsetLocalCenter = V3(CubeWidth) * 0.5f;
    P = V3(0,0,-DepthZ - 10.0f) + OffsetLocalCenter;
    EntityAddTranslation(GameState,Entity,NullEntity(),P,V3(1.0f),3.0f);
    EntityAdd3DRender(GameState,Entity,0,RGB_WHITE);

    ViewLookAt(GameState,CameraP,P);

    GameState->Camera.D = GetMatrixDirection(GameState->ViewRotationMatrix);
    GameState->Camera.Yaw = 0.0f;
    GameState->Camera.Pitch = 0.0f;
}


SCENE_HANDLER(HandleSceneFloor)
{
#if 0
    FreeCameraView(GameState, dP, Yaw, Pitch);
    //if ((dP.x != 0.0f || dP.y != 0.0f || dP.z != 0.0f) && VALID_ENTITY(GameState->Player))
#else
    if (VALID_ENTITY(GameState->Player))
    {
        if (FirstTimePressed(&Input->Controller.Numbers[1]))
        {
            v3 P = GetViewPos(GameState);
            //EntityLookAt(GameState,GameState->Player,P);
        }
        else if (FirstTimePressed(&Input->Controller.Numbers[2]))
        {
            v3 P = GetEntityPos(GameState,GetEntity(GameState,1));
            //EntityLookAt(GameState,GameState->Player,P);
        }
        //dP.y = -dP.z;
        //dP.z = 0.0f;
        EntityAddInput(GameState,GameState->Player,dP);
    }
#endif

    v3 ViewP = GetViewPos(GameState);
    real32 FOV = ToRadians(60.0f);
    real32 WidthOverHeight = (real32)ScreenX / (real32)ScreenY;

    if (Input->Controller.MouseRight.IsPressed)
    {
        if (FirstTimePressed(&Input->Controller.MouseRight))
            GameState->Projection = ProjectionMatrix(FOV*1.5f,WidthOverHeight, 0.1f,200.0f);
    }
    else
    {
        GameState->Projection = ProjectionMatrix(FOV,WidthOverHeight, 0.1f,200.0f);
    }
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

        uint32 TotalSizeEntities = GameState->PermanentArena.CurrentSize;
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

        /* ------------------------ Temporary arenas ----------------------------- */
        Base = (uint8 *)Memory->TransientMemory;
        InitializeArena(&GameState->TemporaryArena,Base, Memory->TransientMemorySize);

        // Small chunks of memory for async work
        // which requires data that will persist
        // multiple frames
        GameState->LimitThreadArenas = 8;
        GameState->ThreadArena = 
            PushArray(&GameState->TemporaryArena, GameState->LimitThreadArenas, thread_memory_arena);

        for (uint32 ThreadBucketIndex = 0;
                    ThreadBucketIndex < GameState->LimitThreadArenas;
                    ++ThreadBucketIndex)
        {
            // TODO: is this too much? should I create multiple size arenas buckets?
            // Do I need so many buckets?
            thread_memory_arena * ThreadArena = GameState->ThreadArena + ThreadBucketIndex;

            uint32 ArenaSize = Megabytes(4);
            Base = PushSize(&GameState->TemporaryArena,ArenaSize);
            memory_arena * Arena = &ThreadArena->Arena;
            InitializeArena(Arena,Base, ArenaSize);

            ThreadArena->InUse = false;
        }

        uint32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        uint32 MeshesArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);


        // TODO: how to properly push vertex inputs to render?
        // Game should tell how much arena?
        // Should buffers be game runtime?
        // or we always reserve a size and then use like we do with cpu memory
        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        CreatePipeline(Memory,GameState);

        v3 WorldCenter = V3(0,0,0);

        WorldInitializeView(GameState, (ToRadians(70.0f)), 
                           ScreenWidth,ScreenHeight, 
                           0.1f, 200.0f, 
                           WorldCenter);

        //GameState->CurrentSceneLoader = LoadSceneFloor;
#if 0
        GameState->CurrentSceneLoader = LoadSceneDebugConvexHull;
        GameState->CurrentSceneHandler = HandleDebugConvexHull;;
#else
        GameState->CurrentSceneLoader = LoadSceneFloor;
        GameState->CurrentSceneHandler = HandleSceneFloor;;

#endif
        GameState->SceneLoaded = false;

        GameState->CameraMode = true;

        GameState->IsInitialized = true;
    }

    if (Input->Reloaded)
    {
        // Game DLL reloaded
        //GameState->CurrentSceneLoader = LoadSceneDebugConvexHull;
        GameState->CurrentSceneLoader = LoadSceneFloor;
        GameState->CurrentSceneHandler = HandleSceneFloor;
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

    if (FirstTimePressed(&Input->Controller.R))
    {
        GameState->SceneLoaded = false;
    }

    if (!GameState->SceneLoaded)
    {
        GameState->TotalEntities = 0;
        GameState->CurrentSceneLoader(GameState,ScreenWidth,ScreenHeight);
        GameState->SceneLoaded = true;
    }

    /* ------------------------- GAME UPDATE ------------------------- */

    v3 InputdP = {};

    if (Input->Controller.Up.IsPressed)
    {
        InputdP.z = 1.0f;
    } 
    if (Input->Controller.Down.IsPressed)
    {
        InputdP.z = -1.0f;
    }
    if (Input->Controller.Left.IsPressed)
    {
        InputdP.x = -1.0f;
    } 
    if (Input->Controller.Right.IsPressed)
    {
        InputdP.x = 1.0f;
    }
    if (Input->Controller.Space.IsPressed)
    {
        InputdP.z = 1.0f;
    }

    real32 Yaw = (MouseX != 0.0f) ? (MouseX / 20.0f) : 0.0f;
    real32 Pitch= (MouseY != 0.0f) ? (MouseY / 20.0f)   : 0.0f;

    /* ------------------------- SCENE HANDLER ------------------------- */
    
    if (GameState->CurrentSceneHandler)
    {
        GameState->CurrentSceneHandler(GameState,Input, InputdP, Yaw, Pitch,ScreenWidth,ScreenHeight);
    }

    /* ----------------------- GAME SYSTEMS UPDATE ---------------------------- */

    // TODO: where should speed go?
    real32 Speed = 3.0f * Input->DtFrame;
    real32 Gravity = 0.0f;//9.8f;

    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        uint32 EntityID = Entity.ID;

        if (EntityHasFlag(GameState,EntityID,component_input))
        {
            v3 dP = GameState->EntitiesInput[EntityID].dP;
            entity_transform * T = GetEntityTransform(GameState,Entity);

            //dP.y = -1.0f; // gravity

            if (LengthSqr(dP) > 1.0f)
            {
                dP = Normalize(dP);
            }

            GameState->EntitiesMomentum[EntityID].dP = V3(dP.x * Speed, dP.y * Speed, dP.z * Speed);

            T->Yaw += Yaw * 5.0f; 
            T->Pitch += Pitch * 5.0f; 

            if (T->Pitch > 89.9f)
            {
                T->Pitch = 89.9f;
                Pitch = 0.0f;
            } 
            else if (T->Pitch < -89.9f)
            {
                T->Pitch = -89.9f;
                Pitch = 0.0f;
            }

            //RotateEntityRight(T,Yaw);
            RotateEntityFill(T, -ToRadians(T->Pitch), ToRadians(T->Yaw), 0);

            EntityRemoveFlag(GameState,Entity,component_input);

            if (dP.x != 0.0f || dP.y != 0.0f || dP.z != 0.0f)
            {
                EntityAddFlag(GameState,Entity,component_momentum);
            }
        }
    }

    // collision
    for (uint32 EntityIndex = 0;
                EntityIndex < GameState->TotalEntities;
                ++EntityIndex)
    {
        entity Entity = GetEntity(GameState,EntityIndex);
        uint32 EntityID = Entity.ID;

        if (EntityHasFlag(GameState,EntityID,component_momentum))
        {
            v3 dP = GameState->EntitiesMomentum[EntityID].dP;
            entity_transform * T = GetEntityTransform(GameState,Entity);
            v3 DesiredP = TestEntityMoveForward(T,&dP);
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

            T->LocalP = VectorMultiply(DesiredP,MaxMove);

            // TODO: explore other non-dampening techniques
            GameState->EntitiesMomentum[EntityID].dP *= 0.85f;//dP - dP*0.5f;

            dP = GameState->EntitiesMomentum[EntityID].dP;

            if (LengthSqr(dP) < 0.001f)
            {
                EntityRemoveFlag(GameState,Entity,component_momentum);
            }

        } // has momentum
    }

    /* ----------------------- Model Transform ------------------------------- */
#if 1
    uint32 TotalEntities = GameState->TotalEntities;
    uint32 MaxThreads;
#if 1
    bool32 Threads[4] = {
        (TotalEntities > 0),
        (TotalEntities > 51),
        (TotalEntities > 101),
        (TotalEntities > 201)
    };
#else
    bool32 Threads[8] = {
        (TotalEntities > 0),
        (TotalEntities > 3000),
        (TotalEntities > 5000),
        (TotalEntities > 8000),
        (TotalEntities > 12000),
        (TotalEntities > 16000),
        (TotalEntities > 20000),
        (TotalEntities > 25000)
    };
#endif
    for (MaxThreads = ArrayCount(Threads);
         MaxThreads > 0;
         --MaxThreads)
    {
        if (Threads[MaxThreads - 1]) break;
    }

    uint32 EntitiesPerThread = (uint32)(TotalEntities * (1.0f / real32(MaxThreads)));
    uint32 RemainingEntities = TotalEntities - (EntitiesPerThread * MaxThreads);

    async_update_entities_model UpdateData[ArrayCount(Threads)] = {};

    for (uint32 ThreadIndex = 0;
            ThreadIndex < MaxThreads;
            ++ThreadIndex)
    {
        UpdateData[ThreadIndex].StartIndex    = ThreadIndex * EntitiesPerThread; // uint32 StartIndex;
        UpdateData[ThreadIndex].GameState     = GameState;                       // game_state * GameState;
        if (ThreadIndex == (MaxThreads - 1))
        {
            UpdateData[ThreadIndex].EntitiesCount = EntitiesPerThread + RemainingEntities;
        }
        else
        {
            UpdateData[ThreadIndex].EntitiesCount = EntitiesPerThread; // uint32 EntitiesCount;
        }

        Memory->AddWorkToWorkQueue(Memory->HighPriorityWorkQueue, AsyncUpdateEntitiesModel, (UpdateData + ThreadIndex));
    }

    Memory->CompleteWorkQueue(Memory->HighPriorityWorkQueue);
#else
    UpdateEntitiesModel(GameState);
#endif

    if (VALID_ENTITY(GameState->Player))
    {
        //v3 P = GetEntityPos(GameState,GameState->Player);
        //v3 D = GetMatrixDirection(GameState->EntitiesTransform[GameState->Player.ID].WorldR);
        //SetViewBehindObject(GameState,P, D, 5.0f, 3.0f);
    }

    /* ------------------------- GAME RENDER ------------------------- */

    v4 ClearColor = V4(0,0,0,1);

    BeginRender(GameState, ClearColor);

    RenderEntities(Memory, GameState);
    
    EndRender(GameState);
}

