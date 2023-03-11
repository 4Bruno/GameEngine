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
CreateWorld2(world * World)
{

    world_pos WC = MapIntoCell(World,WorldPosition(0,0,0),V3(0,0.f,0));
    entity * Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(5.0f));
    EntityAddMesh(Entity,game_asset_mesh_quad,V3(0.0f,0.0f,0.75f), 0.25f);

    WC = WorldPosition(0,0,-2);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(5.0f));
    EntityAddMesh(Entity,game_asset_mesh_quad, V3(1.0f,1.0f,0.0f),0.25f);

    WC = WorldPosition(0,0,-4);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(5.0f));
    EntityAddMesh(Entity,game_asset_mesh_quad,V3(1.0f,0.0f,0.0f),0.25f);

    WC = WorldPosition(0,0,-6);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(10.0f));
    EntityAddMesh(Entity,game_asset_mesh_quad,V3(0.75f,0.75f,0.75f),0.f);

}

void
CreateWorld(world * World)
{

    // Sun == white
    world_pos WC = MapIntoCell(World,WorldPosition(0,0,0),V3(0,10.f,0));
    entity * Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),V3(-1.0f,0,0),3.0f);
    Entity->Color = V3(1.0f,1.0f,1.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube);
    EntityAddCollision(Entity, bounding_volume_sphere);

    WC = WorldPosition(0,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f),V3(-0.5f, -1.2f,0.0f),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube, V3(0.5f,0.5f,0.5f));
    EntityAddCollision(Entity, bounding_volume_sphere);

    WC = WorldPosition(2,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f), V3(-2,0,0),3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(1.0f,0.0f,0.0f));
    EntityAddCollision(Entity, bounding_volume_sphere);

    WC = WorldPosition(-2,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f), V3(2,0,0), 3.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(0.0f,0.0f,1.0f));
    EntityAddCollision(Entity, bounding_volume_sphere);

    WC = WorldPosition(0,2,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f));
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(0.0f,1.0f,0.0f));
    EntityAddCollision(Entity, bounding_volume_sphere);

#if 0 // flat ground
    WC = WorldPosition(0,-5,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(15.0f,0.3f,15.0f),V3(0),0.0f);
    EntityAddMesh(Entity,game_asset_mesh_cube,V3(15.0f,0.3f,15.0f));
    EntityAddCollision(Entity, bounding_volume_aabb);
#endif

    // trees
    WC = WorldPosition(5,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f));
    EntityAddMesh(Entity,game_asset_mesh_tree_001,V3(0.0f,1.0f,0.0f));
    EntityAddCollision(Entity, bounding_volume_sphere);
    WC = WorldPosition(8,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(1.0f));
    EntityAddMesh(Entity,game_asset_mesh_tree_001,V3(0.0f,1.0f,0.0f));
    EntityAddCollision(Entity, bounding_volume_sphere);

#if 1
    WC = WorldPosition(-5,0,0);
    Entity = AddEntity(World, WC);
    EntityAddTranslation(Entity,0,V3(0), V3(5.0f));
    Entity->Color = V3(0.0f,0.0f,0.5f);
    EntityAddMesh(Entity,game_asset_mesh_quad);
    Entity->TextureID = game_asset_texture_test;
    Entity->Material = game_asset_material_texture;
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
graphics_push_texture_data * GraphicsPushTextureData = 0;
graphics_initialize_api   * GraphicsInitializeApi = 0;
graphics_close_api        * GraphicsShutdownAPI = 0;
graphics_wait_for_render  * GraphicsWaitForRender = 0;
graphics_on_window_resize * GraphicsOnWindowResize = 0;
graphics_create_shader_module * GraphicsCreateShaderModule = 0;
graphics_delete_shader_module * GraphicsDeleteShaderModule = 0;
graphics_create_material_pipeline * GraphicsCreateMaterialPipeline = 0;
graphics_destroy_material_pipeline * GraphicsDestroyMaterialPipeline = 0;
graphics_create_transparency_pipeline * GraphicsCreateTransparencyPipeline;

void
ReloadGraphicsAPI(graphics_api * GraphicsAPI)
{
    GraphicsRenderDraw                = GraphicsAPI->GraphicsRenderDraw      ; // 
    GraphicsBeginRenderPass           = GraphicsAPI->GraphicsBeginRenderPass ; // 
    GraphicsEndRenderPass             = GraphicsAPI->GraphicsEndRenderPass   ; // 
    GraphicsPushVertexData            = GraphicsAPI->GraphicsPushVertexData  ; // 
    GraphicsPushTextureData           = GraphicsAPI->GraphicsPushTextureData  ; // 
    GraphicsInitializeApi             = GraphicsAPI->GraphicsInitializeApi   ; // 
    GraphicsShutdownAPI               = GraphicsAPI->GraphicsShutdownAPI     ; // 
    GraphicsWaitForRender             = GraphicsAPI->GraphicsWaitForRender   ; // 
    GraphicsOnWindowResize            = GraphicsAPI->GraphicsOnWindowResize  ; // 
    GraphicsCreateShaderModule        = GraphicsAPI->GraphicsCreateShaderModule;
    GraphicsDeleteShaderModule        = GraphicsAPI->GraphicsDeleteShaderModule;
    GraphicsCreateMaterialPipeline    = GraphicsAPI->GraphicsCreateMaterialPipeline;
    GraphicsDestroyMaterialPipeline   = GraphicsAPI->GraphicsDestroyMaterialPipeline;
    GraphicsCreateTransparencyPipeline= GraphicsAPI->GraphicsCreateTransparencyPipeline;
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
            v3 Color,
            game_asset_id TextureID = (game_asset_id)-1)
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

    EntityAddTranslation(&E,0, LocalP, Scale);
    EntityAddMesh(&E, game_asset_mesh_quad, Color);
    if (TextureID >= game_asset_texture_test)
    {
        E.TextureID = TextureID;
        E.Material = game_asset_material_texture;
    }
    else
    {
        E.Material = game_asset_material_default_no_light;
    }

#if 0
    r32 Rotate = ToRadians(90.0f);
    Quaternion qua;
    Quaternion_fromYRotation(Rotate, &qua);
    Quaternion_multiply(&qua,&E.Transform.LocalR,&E.Transform.LocalR);
#endif
    UpdateTransform(&E.Transform,P);

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
        u32 RenderMemorySize = 4 * MaxRenderUnits * sizeof(render_unit);
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

        u32 TempThreadArenaSize = Megabytes(2);
        // base 1, modulo op to double size
        for (u32 ThreadBucketIndex = 1;
                    ThreadBucketIndex <= GameState->LimitThreadArenas;
                    ++ThreadBucketIndex)
        {
            thread_memory_arena * ThreadArena = 
                GameState->ThreadArena + ThreadBucketIndex - 1;

            Base = PushSize(&GameState->TemporaryArena,TempThreadArenaSize);
            memory_arena * Arena = &ThreadArena->Arena;
            InitializeArena(Arena,Base, TempThreadArenaSize);

            ThreadArena->InUse = false;

            if ((ThreadBucketIndex % 2) == 0)
            {
                TempThreadArenaSize *= 2;
            }
        }

        u32 WorldArenaSize = Megabytes(10);
        Base = PushSize(&GameState->TemporaryArena,WorldArenaSize);
        InitializeArena(&GameState->WorldArena,Base, WorldArenaSize);
        *World = NewWorld(&GameState->WorldArena, 16, 16, 16);
        CreateWorld(World);
        //CreateWorld2(World);
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
        // Renderer depends on assets
        GameState->Assets = NewGameAssets(
                &GameState->TemporaryArena,
                GameState->ThreadArena,
                GameState->LimitThreadArenas);

        GlobalAssets = &GameState->Assets;


        v3 WorldCenterV3 = V3(0,0,0);

        r32 FarView = 500.0f;
        v3 WorldUp = V3(0,1,0);
        GameState->Renderer = 
            NewRenderController(&GameState->RenderArena,MaxRenderUnits,
                                WorldUp, ToRadians(70.0f), ScreenWidth,ScreenHeight, 0.1f, FarView, WorldCenterV3,
                                projection_perspective);

        GameState->RendererUI = 
            NewRenderController(&GameState->RenderArena,100,
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

        // TODO: this is just hack to allow rendering spheres for collision
        GameState->Renderer.Sphere = GetMesh(&GameState->Assets, game_asset_mesh_sphere, 1);

        for (u32 ParticleIndex = 0;
                ParticleIndex < ArrayCount(GameState->Particles);
                ++ParticleIndex)
        {
            particle * Particle = GameState->Particles + ParticleIndex;
            InitializeTransform(&Particle->T, V3(0), V3(0.1f));
            Particle->dP = V3(0.0f);
        }


        GameState->RandomSeed = RandomSeed();

        GameState->IsInitialized = true;
    }

    if (Input->GameDllReloaded)
    {
        ReloadGraphicsAPI(GraphicsAPI);
        GlobalAssets = &GameState->Assets;
        GlobalPlatformMemory = Memory;
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
    u32 TotalEntities = World->ActiveEntitiesCount;

    for (u32 EntityIndex = 0;
             EntityIndex < TotalEntities;
             ++EntityIndex)
    {
        entity * Entity = GameState->World.ActiveEntities + EntityIndex;
        //v3 Gravity = V3(0,-3.0f,0);
        v3 Gravity = V3(0,0.0f,0);
        v3 dP = Entity->Transform.dP * Entity->Transform.Speed + Gravity;
        if (Entity->Transform.Speed == 0)
        {
            dP = V3(0);
        }
        r32 DirMagnitude = LengthSqr(dP);

        if (DirMagnitude > 0.0f)
        { 
            mesh_group * MeshGroup = GetMesh(&GameState->Assets,Entity->MeshID);
            v3 EntityP = Entity->Transform.LocalP;
            // initial movement assume no collision
            v3 EntityNewP = EntityP + Input->DtFrame * dP;

            if (EntityHasFlag(Entity,component_collision))
            {

                if (MeshGroup)
                {
                    r32 EntityRadius = 0;
                    {
                        v3 Scale = Entity->Transform.LocalS;
                        EntityRadius = (Scale.y > Scale.x) ? Scale.y : Scale.x;
                        if (Scale.z > Scale.y)
                        {
                            EntityRadius = Scale.z;
                        }
                    }

                    EntityRadius *= MeshGroup->Sphere.r;
                    sphere EntitySphere = { EntityP, EntityRadius };

                    r32 EarliestCollision = Input->DtFrame;
                    entity * CollisionEntity = 0;
                    r32 CollisionEntityRadius = 0.0f;

                    for (u32 TestEntityIndex = 0;
                            TestEntityIndex < TotalEntities;
                            ++TestEntityIndex)
                    {
                        entity * TestEntity = GameState->World.ActiveEntities + TestEntityIndex;
                        if (TestEntity != Entity)
                        {
                            mesh_group * TestMeshGroup = GetMesh(&GameState->Assets,TestEntity->MeshID);
                            v3 TestEntityP = TestEntity->Transform.LocalP;
                            v3 Scale = TestEntity->Transform.LocalS;
                            r32 TestEntityRadius = (Scale.y > Scale.x) ? Scale.y : Scale.x;
                            if (Scale.z > Scale.y)
                            {
                                TestEntityRadius = Scale.z;
                            }
                            TestEntityRadius *= TestMeshGroup->Sphere.r;
                            sphere TestEntitySphere = { TestEntityP, TestEntityRadius };
                            v3 TestEntitydP = 
                                TestEntity->Transform.dP * TestEntity->Transform.Speed + Gravity;
                            if (TestEntity->Transform.Speed == 0)
                            {
                                TestEntitydP  = V3(0);
                            }
                            //TestEntitydP += (SQR(dt) * TestEntity->Transform.ddP * 0.5f);
                            if (TestMeshGroup)
                            {
                                r32 dtCollision = 0.0f;
                                b32 Collides = 
                                    TestCollisionSpheres(EntitySphere, TestEntitySphere, dP, TestEntitydP, dtCollision);
                                if (Collides && (dtCollision < EarliestCollision))
                                {
                                    CollisionEntity = TestEntity;
                                    EarliestCollision = dtCollision;
                                    CollisionEntityRadius = TestEntityRadius;
                                }
                            }
                        } // Test entity is itself
                    } // for each entity

                    if (CollisionEntity)
                    {
                        EntityNewP = EntityP + EarliestCollision * dP; 

                        v3 TestEntityNewP = 
                            CollisionEntity->Transform.LocalP + 
                            EarliestCollision * CollisionEntity->Transform.dP; 

                        v3 s = Normalize(EntityNewP - TestEntityNewP);
                        v3 v0 = Entity->Transform.dP; // normalized
                        r32 x0 = Inner(s,v0);
                        v3 v0x = s * x0;
                        v3 v0y = v0 - v0x;
                        r32 m0 = EntityRadius;

                        s *= -1;
                        v3 v1 = CollisionEntity->Transform.dP; // normalized
                        r32 x1 = Inner(s,v1);
                        v3 v1x = s * x1;
                        v3 v1y = v1 - v1x;
                        //r32 m1 = 1.0f;
                        r32 m1 = CollisionEntityRadius;

                        r32 OneOverSumMasses = 1.0f / (m0 + m1);
                        Entity->Transform.dP = ( v0x * (m0 - m1) * OneOverSumMasses + 
                                v1x * 2 * m1 * OneOverSumMasses +
                                v0y);

                        CollisionEntity->Transform.dP = ( v1x * (m1 - m0) * OneOverSumMasses + 
                                v0x * 2 * m0 * OneOverSumMasses +
                                v1y);

                        CollisionEntity->Transform.LocalP = TestEntityNewP;
                    }

                } // mesh loaded
            } // entity collides

            Entity->Transform.LocalP = EntityNewP;
        }
    }


    /* ----------------------- Model Transform ------------------------------- */
#if 1
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

    {
        v3 Color = V3(1.0f,1.0f,0.0f);
        r32 BarHeight = 0.1f;
        //UI_PushRect(&GameState->RendererUI,&GameState->DebugOverlay,ui_position_anchored_top,0.5f,BarHeight,Color);
        Color = V3(1.0f,0.0f,0.0f);
        //UI_PushRect(&GameState->RendererUI,&GameState->DebugOverlay,ui_position_anchored_top,0.25f,BarHeight,Color);
        Color = V3(1.0f,0.0f,0.0f);
        //UI_PushRect(&GameState->RendererUI,&GameState->DebugOverlay, ui_position_anchored_bottom,1.0f,1.0f,Color,game_asset_texture_test);
    }

    for (u32 ParticleIndex = 0;
                ParticleIndex < 1; // number of particles spawn per frame
                ++ParticleIndex)
    {

        particle * Particle = GameState->Particles + GameState->NextParticle++;
        if (GameState->NextParticle >= ArrayCount(GameState->Particles))
        {
            GameState->NextParticle = 0;
        }
        v3 Size = V3(RandomBetween(&GameState->RandomSeed,0.1f,0.3f));
        v3 P = V3(RandomBetween(&GameState->RandomSeed,0.0f,0.3f),0,0);
        r32 dPy = RandomBetween(&GameState->RandomSeed,5.0f,9.5f);
        r32 dPx = RandomBetween(&GameState->RandomSeed,-0.5f,0.5f);
        r32 dPz = RandomBetween(&GameState->RandomSeed,-0.5f,0.5f);

        InitializeTransform(&Particle->T, P, Size);
        Particle->dP = V3(dPx,dPy, dPz);
        Particle->ddP = V3(0,-9.8f,0);
        Particle->Color = V3(1.0f,0.3f,0);
    }

#if 0
    u32 TotalParticles = 1;
#else
    u32 TotalParticles = ArrayCount(GameState->Particles);
#endif
    Memset((u8 *)&GameState->ParticleCells[0], 0, sizeof(GameState->ParticleCells));
    v3 GridOrigin = {-0.5f * PARTICLE_CELL_DIM, 0, -0.5f * PARTICLE_CELL_DIM};
    r32 MaxDensityCell = 0;
    for (u32 ParticleIndex = 0;
                ParticleIndex < TotalParticles;
                ++ParticleIndex)
    {
        particle * Particle = GameState->Particles + ParticleIndex;
        v3 P = Particle->T.LocalP - GridOrigin;

        i32 X = TruncateReal32ToInt32(P.x);
        i32 Y = TruncateReal32ToInt32(P.y);
        i32 Z = TruncateReal32ToInt32(P.z);

        if (X < 0) X = 0;
        if (X > PARTICLE_CELL_DIM_MINUS_ONE) X = PARTICLE_CELL_DIM_MINUS_ONE;
        if (Y < 0) Y = 0;
        if (Y > PARTICLE_CELL_DIM_MINUS_ONE) Y = PARTICLE_CELL_DIM_MINUS_ONE;
        if (Z < 0) Z = 0;
        if (Z > PARTICLE_CELL_DIM_MINUS_ONE) Z = PARTICLE_CELL_DIM_MINUS_ONE;

        particle_cell * Cell = &GameState->ParticleCells[X][Y][Z];
        r32 Density = 1.0f;
        Cell->Density += Density;
        MaxDensityCell = MaxDensityCell > Cell->Density ? MaxDensityCell : Cell->Density;
        Cell->VelocityTimesDensity += Density * Particle->dP;
    }

#if 0 // density cubes
    r32 OneOverMaxCellDensity = 1.0f / MaxDensityCell;
    for (u32 X = 0;
                X < PARTICLE_CELL_DIM;
                ++X)
    {
        for (u32 Y = 0;
                Y < PARTICLE_CELL_DIM;
                ++Y)
        {
            for (u32 Z = 0;
                    Z < PARTICLE_CELL_DIM;
                    ++Z)
            {
                particle_cell * Cell = &GameState->ParticleCells[X][Y][Z];
                if (Cell->Density > 0.0f)
                {
                    entity_transform T;
                    v3 P = V3((r32)X - 0.5f*(r32)PARTICLE_CELL_DIM_MINUS_ONE,
                            (r32)Y,
                            (r32)Z - 0.5f*(r32)PARTICLE_CELL_DIM_MINUS_ONE);
                    InitializeTransform(&T, P, V3(0.5f));
                    UpdateTransform(&T, V3(0));
                    r32 DensityOverMax =  Cell->Density * OneOverMaxCellDensity; 
                    v3 Color = V3(DensityOverMax,DensityOverMax,0);
                    PushDraw(Renderer, game_asset_material_default_no_light, &T.WorldT, game_asset_mesh_cube, ASSETS_NULL_TEXTURE, Color,0);
                }
            }
        }
    }
#endif

    i32 NeighborOffsets[27][3] = {
        -1, -1, -1,	 0, -1, -1,	 1, -1, -1,
        -1,  0, -1,	 0,  0, -1,	 1,  0, -1,
        -1,  1, -1,	 0,  1, -1,	 1,  1, -1,
        -1, -1,  0,	 0, -1,  0,	 1, -1,  0,
        -1,  0,  0,	 0,  0,  0,	 1,  0,  0,
        -1,  1,  0,	 0,  1,  0,	 1,  1,  0,
        -1, -1,  1,	 0, -1,  1,	 1, -1,  1,
        -1,  0,  1,	 0,  0,  1,	 1,  0,  1,
        -1,  1,  1,	 0,  1,  1,	 1,  1,  1
    };

    for (u32 ParticleIndex = 0;
                ParticleIndex < TotalParticles;
                ++ParticleIndex)
    {
        particle * Particle = GameState->Particles + ParticleIndex;

        v3 P = Particle->T.LocalP - GridOrigin;

        i32 X = TruncateReal32ToInt32(P.x);
        i32 Y = TruncateReal32ToInt32(P.y);
        i32 Z = TruncateReal32ToInt32(P.z);

        if (X < 1) X = 1;
        if (X > PARTICLE_CELL_DIM_MINUS_TWO) X = PARTICLE_CELL_DIM_MINUS_TWO;
        if (Y < 1) Y = 1;
        if (Y > PARTICLE_CELL_DIM_MINUS_TWO) Y = PARTICLE_CELL_DIM_MINUS_TWO;
        if (Z < 1) Z = 1;
        if (Z > PARTICLE_CELL_DIM_MINUS_TWO) Z = PARTICLE_CELL_DIM_MINUS_TWO;

        particle_cell * CellCenter = &GameState->ParticleCells[X][Y][Z];
        v3 Dispersion = V3(0);
        r32 DispersionCoefficient = 0.1f;
        for (i32 CellX = -1; CellX <= 1; ++CellX)
        {
            for (i32 CellY = -1; CellY <= 1; ++CellY)
            {
                for (i32 CellZ = -1; CellZ <= 1; ++CellZ)
                {
                    particle_cell * NeighborCell = &GameState->ParticleCells[X + CellX][Y + CellY][Z + CellZ];
                    Dispersion += DispersionCoefficient *
                                  (CellCenter->Density - NeighborCell->Density) *
                                  V3((r32)CellX,(r32)CellY,(r32)CellZ);
                }
            }
        }

        v3 ddP = Particle->ddP + Dispersion;

        Particle->T.LocalP += 
                0.5f*SQR(Input->DtFrame) *Input->DtFrame * ddP
            +   Input->DtFrame * Particle->dP; 

        Particle->dP += Input->DtFrame * ddP;

        if (Particle->T.LocalP.y < 0.0f)
        {
            r32 CoeficientRestitution = 0.5f;
            Particle->T.LocalP.y = -Particle->T.LocalP.y;
            Particle->dP.y *= -CoeficientRestitution;  
        }

        Particle->DistToCamera = LengthSqr(CameraP - Particle->T.LocalP);
    }

    // render single mesh with all vertex points versus render quads / particle
    // based on unity meshes < 256 vertex are not worth rendering
#if 1
    const int VerticesInQuad = 6;
    const int TotalParticlePoints = ArrayCount(GameState->Particles) * VerticesInQuad;
    vertex_point * ParticlePoints = GameState->ParticlePoints + 0;

    for (u32 ParticleIndex = 0;
             ParticleIndex < (TotalParticles * VerticesInQuad);
             ParticleIndex += VerticesInQuad)
    {
        particle * Particle = &GameState->Particles[ParticleIndex / VerticesInQuad];

        m4 R = {};
        i32 TooClose = LookAt(&R,Particle->T.LocalP, CameraP, Renderer->WorldUp);
        //if (TooClose) { Assert(0); }
        //
        v3 WorldP = Particle->T.LocalP;
        //Particle->Color = Clamp(Particle->Color + (Input->DtFrame * V3(-0.5f)),0.3f,1.0f);
        Particle->Color = V3(1.0f,0,0);
        
        m4 WorldPM = M4();
        Translate(WorldPM, WorldP);
        m4 Scale = M4(Particle->T.WorldS * 5.0f);
        m4 WorldT = WorldPM * R * Scale;

        // quad vertices
        v3 qv1 = V3(-1,-1,0);
        v3 qv2 = V3(-1,+1,0);
        v3 qv3 = V3(+1,+1,0);
        v3 qv4 = V3(+1,-1,0);

#if 0
        Quaternion_rotate(&Particle->T.WorldR,&qv1,&qv1);
        Quaternion_rotate(&Particle->T.WorldR,&qv2,&qv2);
        Quaternion_rotate(&Particle->T.WorldR,&qv3,&qv3);
        Quaternion_rotate(&Particle->T.WorldR,&qv4,&qv4);
#else
        qv1 = VectorByMatrix(qv1, R) + WorldP;
        qv2 = VectorByMatrix(qv2, R) + WorldP;
        qv3 = VectorByMatrix(qv3, R) + WorldP;
        qv4 = VectorByMatrix(qv4, R) + WorldP;
#endif

        ParticlePoints[ParticleIndex + 0].P = qv2;
        ParticlePoints[ParticleIndex + 1].P = qv1;
        ParticlePoints[ParticleIndex + 2].P = qv4;
        ParticlePoints[ParticleIndex + 3].P = qv2;
        ParticlePoints[ParticleIndex + 4].P = qv4;
        ParticlePoints[ParticleIndex + 5].P = qv3;

        ParticlePoints[ParticleIndex + 0].UV = V2(0,1);
        ParticlePoints[ParticleIndex + 1].UV = V2(0,0);
        ParticlePoints[ParticleIndex + 2].UV = V2(1,0);
        ParticlePoints[ParticleIndex + 3].UV = V2(0,1);
        ParticlePoints[ParticleIndex + 4].UV = V2(1,0);
        ParticlePoints[ParticleIndex + 5].UV = V2(1,1);

        //v3 N = GetMatrixDirection(R);
        v3 N = V3(1.0f,0,0);
        ParticlePoints[ParticleIndex + 0].N = N;
        ParticlePoints[ParticleIndex + 1].N = N;
        ParticlePoints[ParticleIndex + 2].N = N;
        ParticlePoints[ParticleIndex + 3].N = N;
        ParticlePoints[ParticleIndex + 4].N = N;
        ParticlePoints[ParticleIndex + 5].N = N;
    }

    GraphicsPushVertexData(ParticlePoints,sizeof(vertex_point)*TotalParticlePoints,&GameState->ParticlesMesh.GPUVertexBufferBeginOffset);
    m4 ParticleModelT = M4(V3(0.5f));
    v3 ParticleEmiterP = V3(0,0,0);
    Translate(ParticleModelT, ParticleEmiterP);
    m4 Scale = M4(V3(5.0f));
    ParticleModelT = ParticleModelT * Scale;
    if (GameState->ParticlesMesh.Meshes == 0) 
    {
        GameState->ParticlesMesh.Meshes = PushStruct(&GameState->TemporaryArena, mesh);
        GameState->ParticlesMesh.Meshes[0].VertexSize = sizeof(vertex_point)*TotalParticlePoints;
    }
    //PushDrawParticle(Renderer, &ParticleModelT, &GameState->ParticlesMesh, game_asset_texture_particle_01_small, V3(1.0f,0,0), 0);

#else
    for (u32 ParticleIndex = 0;
                ParticleIndex < TotalParticles;
                ++ParticleIndex)
    {
        particle * Particle = GameState->Particles + ParticleIndex;
        //Logn("%f ", Particle->DistToCamera);

        m4 R = {};
        LookAt(&R,Particle->T.LocalP, CameraP, Renderer->WorldUp);
        //LookAt(&Particle->T.LocalR, Particle->T.LocalP, CameraP, Renderer->WorldUp);
        //UpdateTransform(&Particle->T,);
        v3 WorldP = V3(0,0,-3.0f);
        WorldP += Particle->T.LocalP;
        //Particle->Color = Clamp(Particle->Color + (Input->DtFrame * V3(-0.5f)),0.3f,1.0f);
        Particle->Color = V3(1.0f,0,0);
        
        m4 WorldPM = M4();
        Translate(WorldPM, WorldP);
        m4 Scale = M4(Particle->T.WorldS * 5.0f);
        m4 WorldT = WorldPM * R * Scale;

        //PushDraw(Renderer, game_asset_material_default_no_light, &Particle->T.WorldT, game_asset_mesh_cube,ASSETS_NULL_TEXTURE, Particle->Color,0);
        //PushDraw(Renderer, game_asset_material_texture_no_light, &WorldT, game_asset_mesh_quad,game_asset_texture_particle_01_small, Particle->Color,0.0f);
        PushDraw(Renderer, game_asset_material_transparent, &WorldT, game_asset_mesh_quad,game_asset_texture_particle_01_small, Particle->Color,0.0f);
    }
#endif

#if 0
    // test quaternion lookat with single quad
    {
        //entity_transform T;
        //InitializeTransform(&T, V3(0), V3(3.0f));
        //LookAt(&T.LocalR, T.LocalP, CameraP, Renderer->WorldUp);
        //UpdateTransform(&T,V3(0,0,-3.0f));
        m4 R = {};
        v3 LocalP = V3(0);
        LookAt(&R, LocalP, CameraP, Renderer->WorldUp);
        m4 WorldPM = M4();
        Translate(WorldPM, LocalP);
        m4 Scale = M4(V3(3.0f));
        m4 WorldT = WorldPM * R * Scale;
        //PushDraw(Renderer, game_asset_material_texture, &WorldT, game_asset_mesh_quad,game_asset_texture_particle_01_small, V3(1.0f),0);
        PushDraw(Renderer, game_asset_material_default_no_light, &WorldT, game_asset_mesh_quad,ASSETS_NULL_TEXTURE, V3(1.0f),0);
    }
#endif

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

