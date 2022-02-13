#include "game.h"

#include "game_animation.cpp"
//#include "game_ground_generator.cpp"
#include "game_ground_generator_3.cpp"
#include "game_memory.h"


void
FreeCameraView(game_state * GameState, v3 dP, r32 Yaw, r32 Pitch)
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
    GameState->ViewRotationMatrix[0].y = GameState->WorldUp.x;
    GameState->ViewRotationMatrix[1].y = GameState->WorldUp.y;
    GameState->ViewRotationMatrix[2].y = GameState->WorldUp.z;
    RotateFill(&GameState->ViewRotationMatrix, ToRadians(GameState->Camera.Pitch), ToRadians(GameState->Camera.Yaw), 0);
}

b32
FirstTimePressed(game_button * Button)
{
    b32 Result = (Button->IsPressed && !Button->WasPressed);
    return Result;
}

struct scene_data
{
    u32 CurrentNode;
    u32 MaxNode;
    v3 * Max;
    v3 * Min;
    vertex_point * Vertices;
};

SCENE_LOADER(LoadSceneFloor)
{
}


SCENE_HANDLER(HandleSceneFloor)
{
}

#define _FillArray(A,C,V) \
    for (u32 i = 0; \
            i < C; \
            ++i) \
    { \
        A[i] = V; \
    }

#define _FillArrayMember(A,C,M,V) \
    for (u32 i = 0; \
            i < C; \
            ++i) \
    { \
        A[i].M = V; \
    }


void
CreateWorld(world * World)
{

    // Sun == white
    world_pos WC = MapIntoCell(World,WorldPosition(0,0,0),V3(0,-10.f,0));
    entity * Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(1.0f,1.0f,1.0f);
    EntityAddMesh(Entity,Mesh(0));

    WC = WorldPosition(0,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(0.5f,0.5f,0.5f);
    EntityAddMesh(Entity,Mesh(0));

    WC = WorldPosition(2,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(1.0f,0.0f,0.0f);
    EntityAddMesh(Entity,Mesh(0));

    WC = WorldPosition(0,0,-2);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(0.0f,0.0f,1.0f);
    EntityAddMesh(Entity,Mesh(0));

    WC = WorldPosition(0,2,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(0.0f,1.0f,0.0f);
    EntityAddMesh(Entity,Mesh(0));

#if 0
    WC = WorldPosition(0,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(20.0f,0.2f,20.0f),3.0f);
    Entity->Color = V3(0.0f,0.0f,0.5f);
    Entity->WorldP._Offset.y += 1.5f;
    EntityAddMesh(Entity,Mesh(0));
#endif
}
void
GenerateWorld(world * World, world_pos Origin)
{
    r32 RandomDecimal[] = 
    { 
        0.31f,0.39f,0.01f,0.11f,0.66f,0.27f,0.32f,0.56f,0.19f,0.36f,0.12f,0.01f,0.76f,0.37f,0.70f,0.88f,0.16f,0.54f,0.54f,0.81f,0.13f,0.67f,0.90f,0.98f,0.32f,0.84f,0.99f,0.14f,0.02f,0.97f,0.77f,0.93f,0.63f,0.70f,0.78f,0.17f,1.00f,0.19f,0.50f,0.67f,0.47f,0.45f,0.64f,0.06f,0.38f,0.10f,0.68f,0.45f,0.00f,0.63f,0.40f,0.32f,0.72f,0.41f,0.85f,0.15f,0.45f,0.23f,0.34f,0.15f,0.79f,0.51f,0.65f,0.76f,0.84f,0.59f,0.85f,0.35f,0.11f,0.73f,0.62f,0.54f,0.64f,0.14f,0.86f,0.16f,0.83f,0.44f,0.02f,0.73f,0.19f,0.62f,0.96f,0.08f,0.41f,0.17f,0.27f,0.78f,0.18f,0.49f,0.31f,0.35f,0.93f,0.75f,0.21f,0.25f,0.92f,0.05f,0.27f,0.08f,0.16f,0.36f,0.13f,0.46f,0.03f,0.56f,0.10f,0.34f,0.15f,0.69f,0.27f,0.62f,0.60f,0.79f,0.49f,0.05f,0.98f,0.06f,0.16f,0.94f,0.80f,0.91f,0.77f,0.36f,0.58f,0.80f,0.00f,0.57f,0.90f,0.99f,0.39f,0.60f,0.03f,0.86f,0.49f,0.27f,0.25f,0.51f,0.08f,0.31f,0.75f,0.62f,0.48f,0.64f,0.56f,0.12f,0.63f,0.88f,0.32f,0.32f,0.72f,0.01f,0.19f,0.05f,0.58f,0.66f,0.49f,0.02f,0.73f,0.50f,0.83f,0.59f,0.95f,0.83f,0.17f,0.82f,0.29f,0.80f,0.04f,0.47f,0.34f,0.01f,0.97f,0.81f,0.09f,0.75f,0.15f,0.09f,0.38f,0.24f,0.16f,0.83f,0.05f,0.27f,0.37f,0.36f,0.66f,0.42f,0.20f,0.41f,0.70f,0.55f,0.66f,0.95f,0.87f,0.58f,0.94f,0.05f,0.99f,0.38f,0.21f,1.00f,0.75f,0.86f,0.96f,0.67f,0.52f,0.05f,0.85f,0.30f,0.09f,0.23f,0.52f,0.76f,0.91f,0.89f,0.49f,0.07f,0.13f,0.92f,0.63f,0.07f,0.46f,0.18f,0.38f,0.69f,0.90f,0.37f,0.59f,0.21f,0.53f,0.57f,0.47f,0.79f,0.33f,0.31f,0.82f,0.26f,0.37f,0.53f,0.71f,0.77f,0.92f,0.27f,0.35f,0.07f,0.79f,0.06f,0.52f,0.26f,0.44f,0.14f,0.36f,0.62f,0.96f,0.87f,0.33f,0.01f,0.94f,0.41f,0.03f,0.88f,0.30f,0.40f,0.44f,0.74f,0.92f,0.33f,0.66f,0.91f,0.75f,0.08f,0.39f,0.00f,0.88f,0.76f,0.24f,0.72f,0.67f,0.07f,0.58f,0.67f,0.54f,0.52f,0.56f,0.84f,0.33f,0.40f,0.15f,0.07f,0.39f,0.46f,0.23f,0.50f,0.54f,0.38f,0.93f,0.46f,0.58f,0.45f
    };

    i32 BoundaryX = 26;
    i32 BoundaryY = 1;
    i32 BoundaryZ = 26;

    u32 RandomCount = ArrayCount(RandomDecimal);

    for (u32 EntityIndex = 0;
                EntityIndex < 1; 
                ++EntityIndex)
    {
        i32 X = ((EntityIndex * 5) % BoundaryX) + Origin.x;
        i32 Y = ((EntityIndex * 2) % BoundaryY) + Origin.y;
        i32 Z = ((EntityIndex * 3) % BoundaryZ) + Origin.z;
        v3 OffsetP = { 
            RandomDecimal[(EntityIndex * 51) % RandomCount] * 1.5f,
            RandomDecimal[(EntityIndex * 13) % RandomCount] * 1.5f,
            RandomDecimal[(EntityIndex * 31) % RandomCount] * 1.5f
        };
        world_pos P = WorldPosition(X,Y,Z, OffsetP);
        entity * Entity = AddEntity(World, P);
        EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
        v4 ColorDebug = V4(V3(0.0f),1.0f);
        ColorDebug._V[EntityIndex % 3] = 1.0f;

        Entity->Color = ColorDebug.xyz;
        EntityAddMesh(Entity,Mesh(EntityIndex % MAX_MESH_COUNT));
    }

}

void
SetSourceLight(game_state * GameState,world * World)
{
    u32 GroundEntitiesCount = 0;
    for (u32 EntityIndex = 0;
                EntityIndex < World->ActiveEntitiesCount;
                ++EntityIndex)
    {
        entity * Entity = World->ActiveEntities + EntityIndex;
        if (Entity->ID.ID == 1)
        {
            GameState->DebugSourceLightP = GetMatrixPos(Entity->Transform.WorldP);
            break;
        }
    }
}

void
RemoveGroundEntity(world * World)
{
    u32 GroundEntitiesCount = 0;
    for (u32 EntityIndex = 0;
                EntityIndex < World->ActiveEntitiesCount;
                ++EntityIndex)
    {
        entity * Entity = World->ActiveEntities + EntityIndex;
    }
}


debug_cycle * DebugCycles = 0;

extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state * GameState = (game_state *)Memory->PermanentMemory;
    world * World = &GameState->World;

    if (!GameState->IsInitialized)
    {
#if DEBUG
        Assert(Memory->DebugCycle);
        DebugCycles = Memory->DebugCycle;
#endif
        u8 * Base = ((u8 *)Memory->PermanentMemory + sizeof(game_state));
        i32 AvailablePermanentMemory = Memory->PermanentMemorySize - sizeof(game_state);
        InitializeArena(&GameState->PermanentArena,Base, AvailablePermanentMemory);

        /* ------------------------ Temporary arenas ----------------------------- */
        Base = (u8 *)Memory->TransientMemory;
        InitializeArena(&GameState->TemporaryArena,Base, Memory->TransientMemorySize);

        // Small chunks of memory for async work
        // which requires data that will persist
        // multiple frames
        GameState->LimitThreadArenas = 8;
        GameState->ThreadArena = 
            PushArray(&GameState->TemporaryArena, GameState->LimitThreadArenas, thread_memory_arena);

        for (u32 ThreadBucketIndex = 0;
                    ThreadBucketIndex < GameState->LimitThreadArenas;
                    ++ThreadBucketIndex)
        {
            // TODO: is this too much? should I create multiple size arenas buckets?
            // Do I need so many buckets?
            thread_memory_arena * ThreadArena = GameState->ThreadArena + ThreadBucketIndex;

            u32 ArenaSize = Megabytes(4);
            Base = PushSize(&GameState->TemporaryArena,ArenaSize);
            memory_arena * Arena = &ThreadArena->Arena;
            InitializeArena(Arena,Base, ArenaSize);

            ThreadArena->InUse = false;
        }

        u32 WorldArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,WorldArenaSize);
        InitializeArena(&GameState->WorldArena,Base, WorldArenaSize);
        World = &GameState->World;
        (*World) = NewWorld(&GameState->WorldArena, 16, 16, 16);
        CreateWorld(World);
        world_pos WorldCenter = WorldPosition(0,0,0);
        //GenerateWorld(World, WorldCenter);
        GameState->Simulation = PushStruct(&GameState->TemporaryArena,simulation);
        GameState->Simulation->Origin = WorldCenter;
        GameState->Simulation->Dim = V3(80.0f, 10.0f, 80.0f);
        GameState->Simulation->MeshObjTransformCount = 0;

        u32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        u32 MeshesArenaSize = Megabytes(1);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);
        GameState->LimitMeshes = 100;
        GameState->Meshes = PushArray(&GameState->MeshesArena, GameState->LimitMeshes, mesh_group);

        // TODO: how to properly push vertex inputs to render?
        // Game should tell how much arena?
        // Should buffers be game runtime?
        // or we always reserve a size and then use like we do with cpu memory
        GameState->VertexArena = RenderGetMemoryArena();
        GameState->IndicesArena = RenderGetMemoryArena();

        // TODO: Procedural ground generation
        // Pre-allocate in vertex buffer the ground mesh buffer
        // vertex_point = 40 bytes
        // With meshes of 30*30 = 3042 triangles
        // approx 0.11 megabytes
        // 5 MB / 0.12 = 41 ground tiles
        u32 MaxGroundByteSize = Megabytes(5);
        GameState->MaxGroundByteSize = MaxGroundByteSize;
        //u32 VertexBufferBeginOffset = PushMeshSize(&GameState->VertexArena, MaxGroundByteSize,1);

        Assert(World->GroundEntityLimit > 0);
        GameState->GroundMeshLimit = World->GroundEntityLimit;
        GameState->GroundMeshGroup = 
            PushArray(&GameState->MeshesArena,GameState->GroundMeshLimit,mesh_group);

        for (u32 GroundMeshIndex = 0;
                GroundMeshIndex < GameState->GroundMeshLimit;
                ++GroundMeshIndex)
        {
            mesh_group * MeshGroup = GameState->GroundMeshGroup + GroundMeshIndex;
            MeshGroup->Meshes = PushArray(&GameState->MeshesArena, 1,mesh);
            MeshGroup->Loaded = false;
        }

        CreatePipeline(Memory,GameState);

        v3 WorldCenterV3 = V3(0,0,0);

        r32 FarView = 2000.0f;
        WorldInitializeView(GameState, (ToRadians(70.0f)), 
                           ScreenWidth,ScreenHeight, 
                           0.1f, FarView, 
                           WorldCenterV3);

        GameState->CameraMode = true;
        GameState->CameraWorldP = WorldCenter;

        Translate(GameState->ViewMoveMatrix, V3(0,1,0));

        GameState->IsInitialized = true;
    }

    if (Input->Reloaded)
    {
    }

    if (Input->ShaderHasChanged)
    {
        CreatePipeline(Memory,GameState);
        Input->ShaderHasChanged = false;
    }

    r32 MouseX = (r32)Input->Controller.RelMouseX;
    r32 MouseY = (r32)Input->Controller.RelMouseY;
    
    //if (MouseX != 0.0f || MouseY != 0.0f) Log("x: %f y:%f\n",MouseX,MouseY);

    v3 MouseRotation = V3(MouseX,MouseY,0) / Length(V3(MouseX,MouseY,0));

    if (FirstTimePressed(&Input->Controller.R))
    {
        //GameState->Simulation->Origin = WorldPosition(0,0,0);
        Translate(GameState->ViewMoveMatrix, V3(0,0,0));

        GameState->GroundMeshCount = 0;
        RemoveGroundEntity(&GameState->World);
        GameState->GroundMeshGroup[0].Loaded = false;
#if 0
        for (u32 GroundMeshIndex = 0;
                GroundMeshIndex < GameState->GroundMeshLimit;
                ++GroundMeshIndex)
        {
            mesh_group * MeshGroup = GameState->GroundMeshGroup + GroundMeshIndex;
            MeshGroup->Loaded = false;
        }
#endif
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

    r32 Yaw = (MouseX != 0.0f) ? (MouseX / 20.0f) : 0.0f;
    r32 Pitch= (MouseY != 0.0f) ? (MouseY / 20.0f)   : 0.0f;

    // TODO: where should speed go?
    r32 Speed = 30.0f * Input->DtFrame;
    r32 Gravity = 0.0f;//9.8f;

    /* ----------------------- GAME SYSTEMS UPDATE ---------------------------- */
    // Move camera and set simulation in new center
    FreeCameraView(GameState, InputdP * Speed, Yaw, Pitch);
    v3 CameraP = GetViewPos(GameState) + V3(0,1,0);
    CameraP.x = -CameraP.x;
    CameraP.z = -CameraP.z;
    CameraP.y = -CameraP.y;
    //Translate(GameState->ViewMoveMatrix,V3(0));
    GameState->CameraWorldP = MapIntoCell(World,GameState->Simulation->Origin, CameraP);

    //GameState->Simulation->Origin = MapIntoCell(World, GameState->CameraWorldP, V3(0,0,50));
    //Logn("Camera P " STRWORLDP, FWORLDP(GameState->CameraWorldP));
    //Logn("Simul  P " STRWORLDP, FWORLDP(GameState->Simulation->Origin));
    

    /* -------------- GROUND ------------------------ */
    world_pos BeginWorldP = GameState->CameraWorldP;

#if 1
    GenerateGround(Memory,GameState,World,BeginWorldP);
    //GenerateGround2(Memory,GameState,World,BeginWorldP);
#else
    if (GameState->GroundMeshGroup->Loaded == false)
    {
        memory_arena * Arena = &GameState->TemporaryArena;
        world_pos P = WorldPosition(0,0,0);

        BeginTempArena(Arena,1);

        vertex_point * VertexBuffer = (vertex_point *)Arena->Base + Arena->CurrentSize;
        FillBufferTestGround(Arena, P, P, 3);
        
        RenderPushVertexData(Vertices,SizeVertexBuffer, VertexBufferBeginOffset); 

        EndTempArena(Arena,1);

        //TestGroundSingleMeshMultipleVoxel3(GameState,WorldPosition(0,0,0));
        GameState->GroundMeshGroup->Loaded = true;
    }
#endif


    /* -------------- SIMULATION ------------------------ */
    BeginSimulation(World, GameState->Simulation);

#if 0
    entity * Player = World->ActiveEntities + GameState->Simulation->EntityEntries[0].StorageIndex;
    entity * Target = World->ActiveEntities + GameState->Simulation->EntityEntries[1].StorageIndex;

    //ViewLookAt(GameState, Player->Transform.LocalP + V3(3.0f,3.0f,15.f), Target->Transform.LocalP);
    ViewLookAt(GameState, V3(3.0f,9.0f,15.f), Target->Transform.LocalP);
#endif

    /* -------------- INPUT ---------------------------- */
            //  void Quaternion_rotate(Quaternion * q, v3 * v, v3 * output)
    for (u32 EntityIndex = 0;
                //EntityIndex < 0;
                EntityIndex < GameState->World.ActiveEntitiesCount;
                ++EntityIndex)
    {
        entity * Entity = GameState->World.ActiveEntities + EntityIndex;
#if 0
        if (Entity->MeshID.ID == 2)
        {
            Logn(STRP,FP(Entity->Transform.WorldP[3]));
        }
#else
        if (Entity->MeshObjCount > 1)
        {
            if (IS_VALID_MESHOBJ_TRANSFORM_INDEX(Entity->MeshObjTransOcuppancyIndex))
            {
                Quaternion qua;

                Quaternion * LocalR = &Entity->Transform.LocalR;

                r32 Angle = -((25.f / 180.f) * PI) * Input->DtFrame;
                //v3 Move = V3(0,-1.5f,0);
                //Quaternion_multiply(LocalR,Move,Move);
                Quaternion_fromZRotation(Angle, &qua);
                //Quaternion_fromAxisAngle(V3(0,0,1.0f)._V,Angle,&qua);
                Quaternion_multiply(&qua,LocalR,LocalR);

                simulation_mesh_obj_transform_iterator Iterator =
                    BeginSimMeshObjTransformIterator(GameState->Simulation, Entity);

                AdvanceSimMeshObjTransformIterator(&Iterator);

                for (entity_transform * T = Iterator.T;
                        IS_NOT_NULL(T);
                        T = AdvanceSimMeshObjTransformIterator(&Iterator))
                {
                    r32 Rotate = (25.f / 180.f) * PI * Input->DtFrame;
                    Quaternion_fromYRotation(Rotate, &qua);
                    Quaternion_multiply(&qua,&T->LocalR,&T->LocalR);
                }
            }
            break;
        }
#endif
    }

    /* -------------- COLLISION ------------------------ */

    /* ----------------------- Model Transform ------------------------------- */
#if 1
    u32 TotalEntities = World->ActiveEntitiesCount;
    u32 MaxThreads;

    b32 Threads[4] = {
        (TotalEntities > 0),
        (TotalEntities > 51),
        (TotalEntities > 101),
        (TotalEntities > 201)
    };

    for (MaxThreads = ArrayCount(Threads);
         MaxThreads > 0;
         --MaxThreads)
    {
        if (Threads[MaxThreads - 1]) break;
    }

    u32 EntitiesPerThread = (u32)(TotalEntities * (1.0f / r32(MaxThreads)));
    u32 RemainingEntities = TotalEntities - (EntitiesPerThread * MaxThreads);

    async_update_entities_model UpdateData[ArrayCount(Threads)] = {};

    for (u32 ThreadIndex = 0;
            ThreadIndex < MaxThreads;
            ++ThreadIndex)
    {
        UpdateData[ThreadIndex].StartIndex    = ThreadIndex * EntitiesPerThread; // u32 StartIndex;
        UpdateData[ThreadIndex].GameState     = GameState;                       // game_state * GameState;
        if (ThreadIndex == (MaxThreads - 1))
        {
            UpdateData[ThreadIndex].EntitiesCount = EntitiesPerThread + RemainingEntities;
        }
        else
        {
            UpdateData[ThreadIndex].EntitiesCount = EntitiesPerThread; // u32 EntitiesCount;
        }

        Memory->AddWorkToWorkQueue(Memory->HighPriorityWorkQueue, AsyncUpdateEntitiesModel, (UpdateData + ThreadIndex));
    }

    Memory->CompleteWorkQueue(Memory->HighPriorityWorkQueue);

    UpdateGroundModel(World);
#endif

    SetSourceLight(GameState,World);

    /* ------------------------- GAME RENDER ------------------------- */

    v4 ClearColor = V4(0,0,0,1);

    BeginRender(GameState, ClearColor);

    RenderEntities(Memory, GameState);
    
    EndRender(GameState);

   // Logn("Total ground entities %i",GroundEntitiesCount);
}

