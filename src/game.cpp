#include "game.h"

#include "game_animation.cpp"
//#include "game_ground_generator.cpp"
//#include "game_ground_generator.cpp"
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

// rotate example
#if 0
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

#endif
enum ui_position
{
    ui_position_anchored_top,
    ui_position_anchored_bottom
};


void
UI_PushRect(render_controller * Renderer, 
            menu_overlay * Overlay,
            ui_position Position,
            r32 Width, r32 Height,
            v3 Color)
{
    entity E = {};
    //r32 x = (sinf(Input->TimeElapsed) + 1.0f) * 0.5f;
    //r32 x = sinf(Input->TimeElapsed);
    r32 OneOverWidthOverHeight = Renderer->OneOverWidthOverHeight;

    r32 y = 0.0f;
    r32 x = -1.0f;
    r32 Padding = OneOverWidthOverHeight * 0.05f;

    Height = Height * OneOverWidthOverHeight;

    if (Position == ui_position_anchored_top)
    {
        y = OneOverWidthOverHeight - Height;
    }
    else
    {
        y = -OneOverWidthOverHeight;
    }
    //v3 P = V3(-1.0f,-OneOverWidthOverHeight,0);
    v3 P      = V3(x,y,0);
    v3 Scale  = V3(Width,Height,0);
    v3 LocalP = V3(Scale.x, Scale.y, 0);

    EntityAddTranslation(&E,0, LocalP, Scale,0);
    EntityAddMesh(&E, game_asset_mesh_quad, Color);
    E.Material = game_asset_material_default_no_light;
#if 0
    r32 Rotate = ToRadians(90.0f);
    Quaternion qua;
    Quaternion_fromYRotation(Rotate, &qua);
    Quaternion_multiply(&qua,&E.Transform.LocalR,&E.Transform.LocalR);
#endif
    UpdateTransform(&E,P);

    PushDrawEntity(Renderer,&E);
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

        u32 MaxRenderUnits = 4096;
        u32 RenderMemorySize = 2 * MaxRenderUnits * sizeof(render_unit);
        Base = PushSize(&GameState->PermanentArena,RenderMemorySize);
        InitializeArena(&GameState->RenderArena, Base, RenderMemorySize);

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
        *World = NewWorld(&GameState->WorldArena, 16, 16, 16);
        CreateWorld(World);
        world_pos WorldCenter = WorldPosition(0,0,0);
        //GenerateWorld(World, WorldCenter);
        GameState->Simulation = PushStruct(&GameState->TemporaryArena,simulation);
        GameState->Simulation->Origin = WorldCenter;
        GameState->Simulation->Dim = V3(80.0f, 25.0f, 80.0f);
        //GameState->Simulation->MeshObjTransformCount = 0;

#if 0
        u32 ShaderArenaSize = Megabytes(30);
        Base = PushSize(&GameState->TemporaryArena,ShaderArenaSize);
        InitializeArena(&GameState->ShadersArena,Base, ShaderArenaSize);

        u32 MeshesArenaSize = Megabytes(1);
        Base = PushSize(&GameState->TemporaryArena,MeshesArenaSize);
        InitializeArena(&GameState->MeshesArena, Base, MeshesArenaSize);
        GameState->LimitMeshes = 100;
        GameState->Meshes = PushArray(&GameState->MeshesArena, GameState->LimitMeshes, mesh_group);

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
#endif

        v3 WorldCenterV3 = V3(0,0,0);

        r32 FarView = 2000.0f;
        v3 WorldUp = V3(0,1,0);
        GameState->Renderer = 
            NewRenderController(&GameState->RenderArena,MaxRenderUnits,
                                WorldUp, ToRadians(70.0f), ScreenWidth,ScreenHeight, 0.1f, FarView, WorldCenterV3,
                                projection_perspective);

        GameState->RendererUI = 
            NewRenderController(&GameState->RenderArena,MaxRenderUnits,
                                WorldUp, ToRadians(70.0f), ScreenWidth,ScreenHeight, 0.1f, FarView, WorldCenterV3,
                                projection_orthographic);

        GameState->CameraMode = true;
        GameState->CameraWorldP = WorldCenter;

#if 0
        memory_arena * TempArena = &GameState->TemporaryArena;
        BeginTempArena(TempArena,2);
        //GetTexture(GameState,Memory,TempArena, enum_texture_ground_stone_02);

        //TestGroundGPU(Memory,TempArena);
        EndTempArena(TempArena,2);
#endif

        GameState->Assets = NewGameAssets(&GameState->TemporaryArena);

        GlobalAssets = &GameState->Assets;

        GameState->IsInitialized = true;
    }

    if (Input->GameDllReloaded)
    {
        ReloadGraphicsAPI(GraphicsAPI);
        GlobalAssets = &GameState->Assets;
    }

    if (Input->GraphicsDllReloaded)
    {
        ReloadGraphicsAPI(GraphicsAPI);
    }

    if (Input->ShaderHasChanged)
    {
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

    v4 ClearColor = V4(0,0,0,1);
    v4 AmbientLight = V4(0,0,0,0.2f);
    v4 SunlightDirection = V4(GetViewPos(Renderer) + V3(0,1,0),1.0f);
    v4 SunlightColor = V4(1,1,1,1);

    BeginRender(Renderer,ScreenWidth, ScreenHeight);
    BeginRender(&GameState->RendererUI,ScreenWidth, ScreenHeight);

    BeginRenderPass(ClearColor, AmbientLight, SunlightDirection, SunlightColor);

    /* ----------------------- GAME SYSTEMS UPDATE ---------------------------- */
    // Move camera and set simulation in new center
#if 1
    FreeCameraView(Renderer,&GameState->Camera, InputdP * Speed, Yaw, Pitch);
    v3 CameraP = GetViewPos(Renderer);
    //Translate(GameState->ViewMoveMatrix,V3(0));
    GameState->CameraWorldP = MapIntoCell(World,GameState->Simulation->Origin,CameraP);
#else 
    if (GameState->DebugBox)
    {
        ViewLookAt(GameState, V3(-8.0f,12.0f,-5.0f), GameState->DebugBox->Transform.LocalP);
        if (Input->Controller.Up.IsPressed)
        {
            GameState->DebugBox->Transform.LocalP.z += World->GridCellDimInMeters.z;
        } 
        if (Input->Controller.Down.IsPressed)
        {
            GameState->DebugBox->Transform.LocalP.z -= World->GridCellDimInMeters.z;
        }
        if (Input->Controller.Left.IsPressed)
        {
            GameState->DebugBox->Transform.LocalP.x -= World->GridCellDimInMeters.x;
        } 
        if (Input->Controller.Right.IsPressed)
        {
            GameState->DebugBox->Transform.LocalP.x += World->GridCellDimInMeters.x;
        }
    }
#endif

    //GameState->Simulation->Origin = MapIntoCell(World, GameState->CameraWorldP, V3(0,0,50));
    //Logn("Camera P " STRWORLDP, FWORLDP(GameState->CameraWorldP));
    //Logn("Simul  P " STRWORLDP, FWORLDP(GameState->Simulation->Origin));
    

    /* -------------- GROUND ------------------------ */
    world_pos BeginWorldP = GameState->CameraWorldP;

#if 0
    for (u32 i = 0; i < 10;++i)
    {
        v3 LocalP = V3(5,(r32)i * World->GridCellDimInMeters.y,0);
        v3 Scale = World->GridCellDimInMeters * 0.5f;
        entity E = {};
        EntityAddTranslation(&E,0, LocalP, Scale,0);
        UpdateTransform(&E);
        E.Color = V3((i % 3) == 0,((i + 1) % 3) == 0,((i + 2) % 3) == 0);
        E.Transparency = 0.0f;
        PushDrawDebug(Renderer,&E);
    }
#endif

#if 0
    if (GameState->DebugBox)
    {
        world_pos P = MapIntoCell(World,GameState->Simulation->Origin,-GameState->DebugBox->Transform.LocalP);
        GenerateGround(Memory,GameState,World,P);
    }
#else
    //GenerateGround(Memory,GameState,World,BeginWorldP);
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

    // ground chunks matrix transform
    //UpdateGroundModel(World);
#endif

    //SetSourceLight(GameState,World);

    //PushDrawSimulation(Memory, GameState,GameState->Simulation);

    /* ------------------------- GAME END RENDER ------------------------- */

#if 1
    PushDrawSimulation(Renderer,&GameState->World, GameState->Simulation);

    v3 Color = V3(1.0f,1.0f,0.0f);
    r32 BarHeight = 0.1f;
    UI_PushRect(&GameState->RendererUI,&GameState->DebugOverlay,ui_position_anchored_top,0.5f,BarHeight,Color);
    Color = V3(1.0f,0.0f,0.0f);
    UI_PushRect(&GameState->RendererUI,&GameState->DebugOverlay,ui_position_anchored_top,0.25f,BarHeight,Color);

    RenderDraw(Renderer);
    RenderDraw(&GameState->RendererUI);

    //RenderDrawGround(GameState,Renderer, GameState->Simulation);
    EndRender();
#else
    /*UpdateView*/Renderer->ViewTransform = Renderer->ViewRotationMatrix * Renderer->ViewMoveMatrix;
    entity E = {};
    EntityAddTranslation(&E,0, V3(0,0,0), V3(10.0f),0);
    E.Color = V3(1.0f);
    E.Transparency = 0.0f;
    Quaternion qua;

    Quaternion * LocalR = &E.Transform.LocalR;

    r32 Angle = -((25.f / 180.f) * PI) * Input->DtFrame;
    Quaternion_fromZRotation(Angle, &qua);
    Quaternion_multiply(&qua,LocalR,LocalR);
    UpdateTransform(&E);

    m4 MVP = Renderer->Projection * Renderer->ViewTransform * E.Transform.WorldT;
    GroundRenderPasses(&MVP);
#endif
    

   // Logn("Total ground entities %i",GroundEntitiesCount);
}

