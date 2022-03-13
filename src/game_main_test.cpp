#include "game.h"

#include "game_animation.cpp"
#include "game_memory.h"


void
FreeCameraView(render_controller * Renderer, camera * Camera, v3 dP, r32 Yaw, r32 Pitch)
{

    if (dP.z)
    {
        MoveViewForward(Renderer, -dP.z);
    }
    if (dP.x)
    {
        MoveViewRight(Renderer, dP.x);
    }
    if (Yaw)
    {
        Camera->Yaw += -Yaw * 10.0f;
    }
    if (Pitch)
    {
        Camera->Pitch += Pitch * 10.0f;
    }
    if (Camera->Pitch > 89.9f)
    {
        Camera->Pitch = 89.9f;
    }
    else if (Camera->Pitch < -89.9f)
    {
        Camera->Pitch = -89.9f;
    }
    Renderer->ViewRotationMatrix = M4();
    RotateFill(&Renderer->ViewRotationMatrix, ToRadians(Camera->Pitch), ToRadians(Camera->Yaw), 0);
#if 0
    // This makes dizzy rotation
    GameState->ViewRotationMatrix[0].y = GameState->WorldUp.x;
    GameState->ViewRotationMatrix[1].y = GameState->WorldUp.y;
    GameState->ViewRotationMatrix[2].y = GameState->WorldUp.z;
#endif
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
    world_pos WC = MapIntoCell(World,WorldPosition(0,0,0),V3(0,10.f,0));
    entity * Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    Entity->Color = V3(1.0f,1.0f,1.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube);

    WC = WorldPosition(0,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube, V3(0.5f,0.5f,0.5f));

    WC = WorldPosition(2,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(1.0f,0.0f,0.0f));

    WC = WorldPosition(0,0,-2);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(0.0f,0.0f,1.0f));

    WC = WorldPosition(0,2,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(0.0f,1.0f,0.0f));

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
        EntityAddMesh(Entity,(game_asset_id)(game_asset_mesh_begin + 1 + EntityIndex % (u32)(game_asset_mesh_end - game_asset_mesh_begin - 1)));
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



#if DEBUG
debug_cycle * DebugCycles = 0;

#if 0
void
DebugDraw(render_controller * Renderer,v3 LocalP, v3 Scale, v3 Color, r32 Transparency)
{
    entity E = {};
    EntityAddTranslation(&E,0, LocalP, Scale,0);
    UpdateTransform(&E);
    E.Color = Color;
    E.Transparency = Transparency;
    PushDrawDebug(Renderer,&E);
}
#endif
#endif


game_memory * GlobalPlatformMemory = 0;
game_assets * GlobalAssets = 0;

graphics_render_draw      * GraphicsRenderDraw = 0;
graphics_begin_render     * GraphicsBeginRenderPass = 0;
graphics_end_render       * GraphicsEndRenderPass = 0;
graphics_push_vertex_data * GraphicsPushVertexData = 0;
graphics_initialize_api   * GraphicsInitializeApi = 0;
graphics_close_api        * GraphicsShutdownAPI = 0;
graphics_wait_for_render  * GraphicsWaitForRender = 0;
graphics_on_window_resize * GraphicsOnWindowResize = 0;
graphics_create_shader_module * GraphicsCreateShaderModule = 0;
graphics_delete_shader_module * GraphicsDeleteShaderModule = 0;
graphics_create_material_pipeline * GraphicsCreateMaterialPipeline = 0;
graphics_destroy_material_pipeline * GraphicsDestroyMaterialPipeline = 0;


void
ReloadGraphicsAPI(graphics_api * GraphicsAPI)
{
    GraphicsRenderDraw      = GraphicsAPI->GraphicsRenderDraw      ; // 
    GraphicsBeginRenderPass = GraphicsAPI->GraphicsBeginRenderPass ; // 
    GraphicsEndRenderPass   = GraphicsAPI->GraphicsEndRenderPass   ; // 
    GraphicsPushVertexData  = GraphicsAPI->GraphicsPushVertexData  ; // 
    GraphicsInitializeApi   = GraphicsAPI->GraphicsInitializeApi   ; // 
    GraphicsShutdownAPI     = GraphicsAPI->GraphicsShutdownAPI     ; // 
    GraphicsWaitForRender   = GraphicsAPI->GraphicsWaitForRender   ; // 
    GraphicsOnWindowResize  = GraphicsAPI->GraphicsOnWindowResize  ; // 
    GraphicsCreateShaderModule = GraphicsAPI->GraphicsCreateShaderModule;
    GraphicsDeleteShaderModule = GraphicsAPI->GraphicsDeleteShaderModule;
    GraphicsCreateMaterialPipeline = GraphicsAPI->GraphicsCreateMaterialPipeline;
    GraphicsDestroyMaterialPipeline = GraphicsAPI->GraphicsDestroyMaterialPipeline;
}

extern "C"
GAME_API
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((Memory->PermanentMemory) && (Memory->PermanentMemorySize > 0))
    Assert((Memory->TransientMemory) && (Memory->TransientMemorySize > 0))
    Assert((sizeof(game_state) <= Memory->PermanentMemorySize));

    game_state * GameState = (game_state *)Memory->PermanentMemory;
    world * World = &GameState->World;
    render_controller * Renderer = &GameState->Renderer;

#if DEBUG
        Assert(Memory->DebugCycle);
        DebugCycles = Memory->DebugCycle;
#endif

    if (!GameState->IsInitialized)
    {

        GlobalPlatformMemory = Memory;

        ReloadGraphicsAPI(GraphicsAPI);

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

            u32 ArenaSize = Megabytes(6);
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
        GameState->Simulation->Dim = V3(80.0f, 25.0f, 80.0f);
        GameState->Simulation->MeshObjTransformCount = 0;

        u32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        u32 MeshesArenaSize = Megabytes(1);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);
        GameState->LimitMeshes = 100;
        GameState->Meshes = PushArray(&GameState->MeshesArena, GameState->LimitMeshes, mesh_group);

#if 0
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
        }
#endif

        v3 WorldCenterV3 = V3(0,0,0);

        u32 RenderArenaSize = Megabytes(15);
        Base = PushSize(&GameState->TemporaryArena,RenderArenaSize);
        InitializeArena(&GameState->RenderArena,Base, RenderArenaSize);

        r32 FarView = 2000.0f;
        GameState->Renderer = 
            NewRenderController(V3(0,1,0), ToRadians(70.0f),
                                ScreenWidth,ScreenHeight, 
                                0.1f, FarView,
                                WorldCenterV3);

        GameState->CameraMode = true;
        GameState->CameraWorldP = WorldCenter;

#if 1
        memory_arena * TempArena = &GameState->TemporaryArena;
        BeginTempArena(TempArena,2);
        //TestGroundGPU(Memory,TempArena);
        EndTempArena(TempArena,2);
#endif
        GameState->Assets = NewGameAssets(&GameState->TemporaryArena);

        GlobalAssets = &GameState->Assets;

        GameState->IsInitialized = true;
    }

    if (Input->GameDllReloaded)
    {
    }

    if (Input->GraphicsDllReloaded)
    {
        ReloadGraphicsAPI(GraphicsAPI);
    }

    if (Input->ShaderHasChanged)
    {
        //DestroyPipelines(GameState,Renderer);
        //CreateAllPipelines(GameState, Memory);
        Input->ShaderHasChanged = false;
    }

    r32 MouseX = (r32)Input->Controller.RelMouseX;
    r32 MouseY = (r32)Input->Controller.RelMouseY;
    
    //if (MouseX != 0.0f || MouseY != 0.0f) Log("x: %f y:%f\n",MouseX,MouseY);

    v3 MouseRotation = V3(MouseX,MouseY,0) / Length(V3(MouseX,MouseY,0));

    if (FirstTimePressed(&Input->Controller.R))
    {
        //GameState->Simulation->Origin = WorldPosition(0,0,0);
        Translate(Renderer->ViewMoveMatrix, V3(0,0,0));

#if 0
        GameState->GroundMeshCount = 0;
        RemoveGroundEntity(&GameState->World);
        GameState->GroundMeshGroup[0].Loaded = false;
#endif
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

    /* ------------------------- GAME BEGIN RENDER ------------------------- */
#if 1
    FreeCameraView(Renderer,&GameState->Camera, InputdP * Speed, Yaw, Pitch);
    v3 CameraP = GetViewPos(Renderer);
    //Translate(GameState->ViewMoveMatrix,V3(0));
    GameState->CameraWorldP = MapIntoCell(World,GameState->Simulation->Origin,CameraP);
#else
    ViewLookAt(Renderer, V3(10.f,10.f,-5.f), V3(0,0,0));
#endif

    //GameState->Simulation->Origin = MapIntoCell(World, GameState->CameraWorldP, V3(0,0,50));
    //Logn("Camera P " STRWORLDP, FWORLDP(GameState->CameraWorldP));
    //Logn("Simul  P " STRWORLDP, FWORLDP(GameState->Simulation->Origin));
    
    /*UpdateView*/Renderer->ViewTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
    entity E = {};
    EntityAddTranslation(&E,0, V3(0,0,0), V3(1.0f),0);
    E.Color = V3(1.0f);
    E.Transparency = 0.0f;
    Quaternion qua;

    Quaternion * LocalR = &E.Transform.LocalR;

    r32 Angle = -((25.f / 180.f) * PI) * Input->DtFrame;
    Quaternion_fromZRotation(Angle, &qua);
    Quaternion_multiply(&qua,LocalR,LocalR);
    UpdateTransform(&E);

    m4 MVP = Renderer->Projection * Renderer->ViewTransform * E.Transform.WorldT;
    //GroundRenderPasses(&MVP);
}

