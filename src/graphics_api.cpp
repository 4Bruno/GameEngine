#include "vulkan_initializer.cpp"
//#include "vulkan_ground_generation.cpp"

#if DEBUG
debug_cycle * DebugCycles = 0;
#endif

GRAPHICS_INITIALIZE_API(InitializeAPI)
{
#if DEBUG
    DebugCycles = DebugCyclesArray;
#endif
    b32 ErrorCode = InitializeVulkan(Width, Height, PlatformWindow, EnableValidationLayer,GetInstanceProcAddr);

    return ErrorCode;
}

GRAPHICS_RENDER_DRAW(RenderDraw)
{
    START_CYCLE_COUNT(render_entities);

    gpu_memory_mapping_result ObjectsMapResult = BeginObjectMapping(Renderer->UnitsCount);

    if (!ObjectsMapResult.Success)
    {
        Assert(0);
        return;
    }

    GPUObjectData * ObjectData = (GPUObjectData *)ObjectsMapResult.BeginAddress;

    for (u32 UnitIndex = 0;
            UnitIndex < Renderer->UnitsCount;
            ++UnitIndex)
    {
        render_unit * Unit = Renderer->Units + UnitIndex;

        m4 MVP = Renderer->Projection * Renderer->ViewTransform * Unit->ModelTransform;

        ObjectData->ModelMatrix = Unit->ModelTransform;
        ObjectData->MVP = MVP;
        ObjectData->Color = Unit->Color;

        ++ObjectData;
    }

    EndObjectsArena();

    i32 LastMaterialPipelineIndex = -1;
    mesh_group * LastMeshGroup = 0;

    u32 ObjectInstance = ObjectsMapResult.Instance;

    for (u32 UnitIndex = 0;
            UnitIndex < Renderer->UnitsCount;
            ++UnitIndex)
    {
        render_unit * Unit = Renderer->Units + UnitIndex;

        mesh_group * MeshGroup = Unit->MeshGroup;
        i32 MaterialPipelineIndex = Unit->MaterialPipelineIndex;

        if (MaterialPipelineIndex != LastMaterialPipelineIndex)
        {
            RenderSetPipeline(MaterialPipelineIndex);
            LastMaterialPipelineIndex = MaterialPipelineIndex;
        }

        // hardcoded only first mesh
        u32 MeshSize = MeshGroup->Meshes[0].VertexSize / sizeof(vertex_point);

        if (LastMeshGroup != MeshGroup)
        {
            RenderBindMesh(MeshSize, MeshGroup->GPUVertexBufferBeginOffset);
            LastMeshGroup = MeshGroup;
        }

        RenderDrawObject(MeshSize,ObjectInstance);
        ++ObjectInstance;
    }

    END_CYCLE_COUNT(render_entities);
}

GRAPHICS_BEGIN_RENDER(BeginRenderPass)
{
    WaitForRender();

    RenderBeginPass(ClearColor);

    RenderPushSimulationData(SimData);
}

GRAPHICS_END_RENDER(EndRenderPass)
{
    VkCommandBuffer cmd = GetCurrentFrame()->PrimaryCommandBuffer;
    u32 SwapchainImageIndex = GlobalVulkan.CurrentSwapchainImageIndex;

    vkCmdEndRenderPass(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo SubmitInfo     = {};
    SubmitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;         // VkStructureType   sType;
    SubmitInfo.pNext                = 0;                                     // Void * pNext;
    SubmitInfo.waitSemaphoreCount   = 1;                                     // u32_t   waitSemaphoreCount;
    SubmitInfo.pWaitSemaphores      = &GetCurrentFrame()->ImageAvailableSemaphore; // Typedef * pWaitSemaphores;
    SubmitInfo.pWaitDstStageMask    = &WaitStage;                            // Typedef * pWaitDstStageMask;
    SubmitInfo.commandBufferCount   = 1;                                     // u32_t   commandBufferCount;
    SubmitInfo.pCommandBuffers      = &cmd;                                  // Typedef * pCommandBuffers;
    SubmitInfo.signalSemaphoreCount = 1;                                     // u32_t   signalSemaphoreCount;
    SubmitInfo.pSignalSemaphores    = &GetCurrentFrame()->RenderSemaphore;         // Typedef * pSignalSemaphores;

    VK_CHECK(vkQueueSubmit(GlobalVulkan.GraphicsQueue, 1, &SubmitInfo, GetCurrentFrame()->RenderFence));

    VkPresentInfoKHR PresentInfo;
    PresentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; // VkStructureType   sType;
    PresentInfo.pNext              = 0; // Void * pNext;
    PresentInfo.waitSemaphoreCount = 1; // u32_t   waitSemaphoreCount;
    PresentInfo.pWaitSemaphores    = &GetCurrentFrame()->RenderSemaphore; // Typedef * pWaitSemaphores;
    PresentInfo.swapchainCount     = 1; // u32_t   swapchainCount;
    PresentInfo.pSwapchains        = &GlobalVulkan.Swapchain; // Typedef * pSwapchains;
    PresentInfo.pImageIndices      = &SwapchainImageIndex; // Typedef * pImageIndices;
    PresentInfo.pResults           = 0; // Typedef * pResults;

    VK_CHECK(vkQueuePresentKHR(GlobalVulkan.GraphicsQueue, &PresentInfo));

    ++GlobalVulkan._CurrentFrameData;

    return 0;
}

void
EndRender(render_controller * Renderer)
{
    EndRenderPass();

    Renderer->UnitsCount = 0;
}


#if 0

void
DestroyPipelines(render_controller * Renderer)
{
    for (i32 i = 0; i < ArrayCount(Renderer->Pipelines); ++i)
    {
        Renderer->Pipelines[i] = -1;
    }
    RenderFreeShaders();
    VulkanDestroyPipeline();
}
pipeline_creation_result
CreatePipeline(game_memory * Memory,memory_arena * TempArena,render_controller * Renderer,material_type Material)
{
    Assert((i32)Material < ArrayCount(Renderer->Pipelines));
    const char * ShadersVertexFile[] = {
        "shaders\\triangle.vert",
        "shaders\\triangle_text.vert",
    };
    const char * ShadersFragmentFile[] = {
        "shaders\\triangle.frag",
        "shaders\\triangle_text.frag",
    };

    BeginTempArena(TempArena,1);

    Renderer->VertexShaders[Material] = LoadShader(Memory,TempArena,ShadersVertexFile[Material]);
    Renderer->FragmentShaders[Material] = LoadShader(Memory,TempArena,ShadersFragmentFile[Material]);

    pipeline_creation_result PipelineResult = RenderCreatePipeline(Renderer->VertexShaders[Material], Renderer->FragmentShaders[Material]);

    // TODO: destroy shaders after pipeline creation
    Assert(PipelineResult.Success);

    Renderer->Pipelines[Material] = PipelineResult.Pipeline;

    EndTempArena(TempArena,1);

    return PipelineResult;
}
void
CreateAllPipelines(game_memory * Memory,render_controller * Renderer, memory_arena * TemporaryArena)
{
    for (i32 i = 0; i < material_type_count; ++i)
    {
        pipeline_creation_result Result =  CreatePipeline(Memory,TemporaryArena,Renderer,(material_type)i);
        Renderer->Pipelines[i] = Result.Pipeline;
    }
}
void
TestGroundGPU(game_memory * Memory,memory_arena * TempArena)
{
    file_contents GetFileResult = GetFileContents(Memory, TempArena,"shaders\\ground_build_densities.vert");
    file_contents GetFileResult2 = GetFileContents(Memory, TempArena, "shaders\\ground.frag");
    file_contents GetFileResult3 = GetFileContents(Memory, TempArena, "shaders\\ground_build_densities.geom");

    file_contents Files[] = {
        GetFileResult,
        GetFileResult3,
        GetFileResult2
    };
    if (GetFileResult.Success)
    {
        GroundInitPipelines(Files,ArrayCount(Files));
        // data lives on gpu side now
        TempArena->CurrentSize -= GetFileResult.Size;
    }

}


void
GetTexture(game_state * GameState,game_memory * Memory,memory_arena * Arena, enum_textures TextureID)
{
    /*
     *
     * Reading a 2MB jpeg requires about 10 MB of temporary memory to do the entire process
     * without cleaning up in between
     */
    Assert(TextureID >= 1);
    const char * PathTextures[] = {
        "..\\..\\assets\\ground_stone_01.jpg",
        "..\\..\\assets\\ground_stone_02.jpg"
    };

    file_contents GetFileResult = GetFileContents(Memory, Arena,PathTextures[TextureID - 1]);
    if (GetFileResult.Success)
    {
        i32 x,y,comp;
        i32 DesiredChannels = 4;
        stbi_uc * Data = 
            stbi_load_from_memory(Arena,GetFileResult.Base, GetFileResult.Size, &x,&y, &comp, DesiredChannels);
        u32 Size = x * y * DesiredChannels;
        VulkanPushTexture(Data, x, y, 0,DesiredChannels);
    }
    else
    {
        Assert(0); // Expecting texture to be loaded
    }
}
#endif


GRAPHICS_CLOSE_API(ShutdownAPI)
{
    CloseVulkan();
}

