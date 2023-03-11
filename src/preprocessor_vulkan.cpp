#include "preprocessor_assets.h"
#define VK_NO_PROTOTYPES
#include "vulkan\vulkan_core.h"

#define VULKAN_CREATE_SURFACE(name) i32 name(void * SurfaceData, void * pfnOSSurface, VkInstance Instance, VkSurfaceKHR * Surface)
typedef VULKAN_CREATE_SURFACE(vulkan_create_surface);

#define VK_FAILS(result) (result != VK_SUCCESS)
#define VK_PASSES(result) (result == VK_SUCCESS)
#define VK_VALID_HANDLE(handle) ((handle) != VK_NULL_HANDLE)
#define VK_INVALID_HANDLE(handle) ((handle) == VK_NULL_HANDLE)

#define VK_GLOBAL_LEVEL_FN( fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(0,#fun)
// watch out for the convention name of instance as "Instance"
#define VK_INSTANCE_LEVEL_FN(Instance, fun ) fun = (PFN_##fun)vkGetInstanceProcAddr(Instance,#fun)
#define VK_DEVICE_LEVEL_FN(Device, fun )   fun = (PFN_##fun)vkGetDeviceProcAddr(Device,#fun)

#define RENDER_VERSION      VK_MAKE_VERSION(1,0,0)
#define ENGINE_VERSION      VK_MAKE_VERSION(1,0,0)
/*
 * Why vulkan version?
 * - 1.1.0+ you can pass negative height to surface to use bottom-left coord system
 * - 1.3.+ query image requirements without creating images
 */
#define VULKAN_API_VERSION  VK_MAKE_API_VERSION(0,1,3,0)

#define VK_MEMORY_GPU                       (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
#define VK_MEMORY_CPU_TO_GPU_PREFERRED      (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
#define VK_MEMORY_CPU_TO_GPU_HOST_VISIBLE   (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
#define VK_MEMORY_LARGE_TARGETS_NOT_STORED  (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)

// vulkan is verbose
// all func calls that return VkResult follow this logic
#define VK_CHECK(FunCall) \
    if (VK_FAILS(FunCall)) \
    { \
        Log("Error during function %s\n",#FunCall); \
        return 1; \
    } \

struct vk_version
{
    u32 Major,Minor,Patch;
};

/*
 * @fpVulkanCreateSurface       Pointer to OS specific function which creates the VkSurface handle 
 * @SurfaceData                 Pointer to OS specific data needed in fpVulkanCreateSurface
 * @VkKHROSSurfaceExtensionName Name of the khronos OS specific surface name which is needed to check support
 * @OSSurfaceFuncName           Name of the OS specific instance function name which we need the proc address
 */
struct graphics_platform_window
{
    vulkan_create_surface * pfnVulkanCreateSurface;
    void *                  SurfaceData;
    const char *            VkKHROSSurfaceExtensionName;
    const char *            OSSurfaceFuncName;
};

PFN_vkGetInstanceProcAddr                     vkGetInstanceProcAddr                   = 0;

// INSTANCE VULKAN
PFN_vkCreateInstance                          vkCreateInstance                        = 0;
PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties  = 0;
PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties      = 0;


// PHYSICAL DEVICES
// SWAP CHAIN API
PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR                       = 0;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR      = 0;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = 0;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR      = 0;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = 0;
PFN_vkGetPhysicalDeviceMemoryProperties       vkGetPhysicalDeviceMemoryProperties       = 0;
PFN_vkGetPhysicalDeviceFormatProperties       vkGetPhysicalDeviceFormatProperties       = 0;

PFN_vkCreateSwapchainKHR                      vkCreateSwapchainKHR                      = 0;
PFN_vkDestroySwapchainKHR                     vkDestroySwapchainKHR                     = 0;
PFN_vkGetSwapchainImagesKHR                   vkGetSwapchainImagesKHR                   = 0;
PFN_vkAcquireNextImageKHR                     vkAcquireNextImageKHR                     = 0;
PFN_vkQueuePresentKHR                         vkQueuePresentKHR                         = 0;


PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices                = 0;
PFN_vkEnumerateDeviceExtensionProperties      vkEnumerateDeviceExtensionProperties      = 0;
PFN_vkGetPhysicalDeviceProperties             vkGetPhysicalDeviceProperties             = 0;
PFN_vkGetPhysicalDeviceFeatures               vkGetPhysicalDeviceFeatures               = 0;
PFN_vkGetPhysicalDeviceFeatures2              vkGetPhysicalDeviceFeatures2              = 0;
PFN_vkGetPhysicalDeviceQueueFamilyProperties  vkGetPhysicalDeviceQueueFamilyProperties  = 0;
PFN_vkCreateDevice                            vkCreateDevice                            = 0;
PFN_vkGetDeviceProcAddr                       vkGetDeviceProcAddr                       = 0;
PFN_vkDestroyInstance                         vkDestroyInstance                         = 0;
PFN_vkCreateCommandPool                       vkCreateCommandPool                       = 0;
PFN_vkDestroyCommandPool                      vkDestroyCommandPool                      = 0;
PFN_vkResetCommandPool                        vkResetCommandPool                        = 0;
PFN_vkResetCommandBuffer                      vkResetCommandBuffer                      = 0;
PFN_vkAllocateCommandBuffers                  vkAllocateCommandBuffers                  = 0;  
PFN_vkFreeCommandBuffers                      vkFreeCommandBuffers                      = 0;  
PFN_vkBeginCommandBuffer                      vkBeginCommandBuffer                      = 0;
PFN_vkEndCommandBuffer                        vkEndCommandBuffer                        = 0;
PFN_vkCmdPushConstants                        vkCmdPushConstants                        = 0;
PFN_vkCmdCopyBuffer                           vkCmdCopyBuffer                           = 0;
PFN_vkCmdCopyBufferToImage                    vkCmdCopyBufferToImage                    = 0;
PFN_vkCmdBeginRenderPass                      vkCmdBeginRenderPass                      = 0;
PFN_vkCmdEndRenderPass                        vkCmdEndRenderPass                        = 0;
PFN_vkCmdPipelineBarrier                      vkCmdPipelineBarrier                      = 0;
PFN_vkCmdClearColorImage                      vkCmdClearColorImage                      = 0;
PFN_vkCmdBindPipeline                         vkCmdBindPipeline                         = 0;
PFN_vkCmdDraw                                 vkCmdDraw                                 = 0;
PFN_vkCmdDrawIndexed                          vkCmdDrawIndexed                          = 0;
PFN_vkCmdBindVertexBuffers                    vkCmdBindVertexBuffers                    = 0;
PFN_vkCmdBindIndexBuffer                      vkCmdBindIndexBuffer                      = 0;
PFN_vkQueueSubmit                             vkQueueSubmit                             = 0;
PFN_vkQueueWaitIdle                           vkQueueWaitIdle                           = 0;
PFN_vkCreateShaderModule                      vkCreateShaderModule                      = 0;
PFN_vkDestroyShaderModule                     vkDestroyShaderModule                     = 0;
PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT            = 0;
PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT           = 0;
PFN_vkSetDebugUtilsObjectNameEXT              vkSetDebugUtilsObjectNameEXT              = 0;

//  LOGICAL DEVICE
PFN_vkGetDeviceQueue                          vkGetDeviceQueue                          = 0;
PFN_vkDestroyDevice                           vkDestroyDevice                           = 0;
PFN_vkDeviceWaitIdle                          vkDeviceWaitIdle                          = 0;
PFN_vkCreateFence                             vkCreateFence                             = 0;
PFN_vkDestroyFence                            vkDestroyFence                            = 0;
PFN_vkWaitForFences                           vkWaitForFences                           = 0;
PFN_vkResetFences                             vkResetFences                             = 0;
PFN_vkCreateSemaphore                         vkCreateSemaphore                         = 0;
PFN_vkDestroySemaphore                        vkDestroySemaphore                        = 0;
PFN_vkCreateRenderPass                        vkCreateRenderPass                        = 0;
PFN_vkDestroyRenderPass                       vkDestroyRenderPass                       = 0;
PFN_vkCreateImageView                         vkCreateImageView                         = 0;
PFN_vkCreateImage                             vkCreateImage                             = 0;
PFN_vkCreateSampler                           vkCreateSampler                           = 0;
PFN_vkDestroySampler                          vkDestroySampler                          = 0;
PFN_vkDestroyImageView                        vkDestroyImageView                        = 0;
PFN_vkCreateFramebuffer                       vkCreateFramebuffer                       = 0; 
PFN_vkDestroyFramebuffer                      vkDestroyFramebuffer                      = 0; 
PFN_vkCreateGraphicsPipelines                 vkCreateGraphicsPipelines                 = 0;
PFN_vkCreatePipelineLayout                    vkCreatePipelineLayout                    = 0;
PFN_vkDestroyPipeline                         vkDestroyPipeline                         = 0;
PFN_vkDestroyPipelineLayout                   vkDestroyPipelineLayout                   = 0;

PFN_vkCreateDescriptorSetLayout               vkCreateDescriptorSetLayout               = 0;
PFN_vkDestroyDescriptorSetLayout              vkDestroyDescriptorSetLayout              = 0;
PFN_vkAllocateDescriptorSets                  vkAllocateDescriptorSets                  = 0;  
PFN_vkUpdateDescriptorSets                    vkUpdateDescriptorSets                    = 0;
PFN_vkCreateDescriptorPool                    vkCreateDescriptorPool                    = 0;
PFN_vkDestroyDescriptorPool                   vkDestroyDescriptorPool                   = 0;
PFN_vkCmdBindDescriptorSets                   vkCmdBindDescriptorSets                   = 0;

PFN_vkAllocateMemory                          vkAllocateMemory                          = 0;
PFN_vkFreeMemory                              vkFreeMemory                              = 0;
PFN_vkCreateBuffer                            vkCreateBuffer                            = 0;
PFN_vkDestroyBuffer                           vkDestroyBuffer                           = 0;
PFN_vkDestroyImage                            vkDestroyImage                            = 0;
PFN_vkGetBufferMemoryRequirements             vkGetBufferMemoryRequirements             = 0;
PFN_vkGetDeviceImageMemoryRequirements        vkGetDeviceImageMemoryRequirements        = 0;
PFN_vkGetImageMemoryRequirements              vkGetImageMemoryRequirements              = 0;
PFN_vkMapMemory                               vkMapMemory                               = 0;
PFN_vkUnmapMemory                             vkUnmapMemory                             = 0;
PFN_vkBindBufferMemory                        vkBindBufferMemory                        = 0;
PFN_vkBindImageMemory                         vkBindImageMemory                         = 0;

PFN_vkCmdSetDepthTestEnable                   vkCmdSetDepthTestEnable                   = 0;
PFN_vkCmdNextSubpass                          vkCmdNextSubpass                          = 0;


b32
VulkanInstanceSupportsSurface(const char * VulkanSurfaceExtensionNameOS)
{
    u32 TotalInstanceExtensionProp = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, 0));

    VkExtensionProperties InstanceExtensionProp[20] = {};
    Assert(ArrayCount(InstanceExtensionProp) >= TotalInstanceExtensionProp);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, &InstanceExtensionProp[0]));

    b32 SurfaceSupport       = false;
    b32 SurfaceSupportOS     = false;

    for (u32 i = 0;
        i < TotalInstanceExtensionProp;
        ++i)
    {
        if (!SurfaceSupport && 
            (strcmp(InstanceExtensionProp[i].extensionName,VK_KHR_SURFACE_EXTENSION_NAME) == 0))
        {
            SurfaceSupport = true;
        }
        else if (!SurfaceSupportOS
                && (strcmp(InstanceExtensionProp[i].extensionName,VulkanSurfaceExtensionNameOS) == 0))
        {
            SurfaceSupportOS = true;
        }
    }

    return (SurfaceSupport && SurfaceSupportOS);
}

i32
VulkanGetInstance(b32 EnableValidationLayer, 
                  const char * VkKHROSSurfaceExtensionName,
                  void ** pfnOSSurface, const char * OSSurfaceFuncName,
                  PFN_vkGetInstanceProcAddr VulkanInstanceProcAddress)
{
    vkGetInstanceProcAddr = VulkanInstanceProcAddress;

    if (!vkGetInstanceProcAddr)
    {
        Log("Global Instance Proc address Function is not valid\n");
        return 1;
    }

    // VULKAN INSTANCE FUNCTIONS
    VK_GLOBAL_LEVEL_FN(vkCreateInstance);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceExtensionProperties);
    VK_GLOBAL_LEVEL_FN(vkEnumerateInstanceLayerProperties);

    VkApplicationInfo AppInfo = {};
    AppInfo.sType                 = VK_STRUCTURE_TYPE_APPLICATION_INFO; // VkStructureType   sType;
    AppInfo.pNext                 = 0; // Void * pNext;
    AppInfo.pApplicationName      = "Render Engine"; // Char_S * pApplicationName;
    AppInfo.applicationVersion    = RENDER_VERSION; // u32_t   applicationVersion;
    AppInfo.pEngineName           = ""; // Char_S * pEngineName;
    AppInfo.engineVersion         = ENGINE_VERSION; // u32_t   engineVersion;
    AppInfo.apiVersion            = VULKAN_API_VERSION; // u32_t   apiVersion;
    
    const char * ExtensionsRequired[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VkKHROSSurfaceExtensionName,
#if DEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#endif
    };

    if (!VulkanInstanceSupportsSurface(VkKHROSSurfaceExtensionName))
    {
        Logn("Surface (%s) not supported",VkKHROSSurfaceExtensionName);
    }

    u32 TotalInstanceExtensionProp = 0;
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, 0));

    VkExtensionProperties InstanceExtensionProp[20] = {};
    Assert(ArrayCount(InstanceExtensionProp) >= TotalInstanceExtensionProp);
    VK_CHECK(vkEnumerateInstanceExtensionProperties(0, &TotalInstanceExtensionProp, &InstanceExtensionProp[0]));

    b32 AnyInstanceExtPropNotFound = false;

    for (u32 ExtReqIndex = 1;// SURFACE IS CHECKED WITH SPECIAL FUNCTION
             ExtReqIndex < ArrayCount(ExtensionsRequired);
             ++ExtReqIndex)
    {
        u32 InstanceExtPropIndex = 0;
        for (InstanceExtPropIndex = 0; 
            InstanceExtPropIndex < TotalInstanceExtensionProp;
            ++InstanceExtPropIndex)
        {
            if ((strcmp(InstanceExtensionProp[InstanceExtPropIndex].extensionName,ExtensionsRequired[ExtReqIndex]) == 0))
            {
                break;
            }
        }
        if (InstanceExtPropIndex == TotalInstanceExtensionProp)
        {
            Logn("Instance extension property (%s) not found.", ExtensionsRequired[ExtReqIndex]);
            AnyInstanceExtPropNotFound = true;
        }
    }

    Assert(!AnyInstanceExtPropNotFound);
    if (AnyInstanceExtPropNotFound)
    { 
        return 1;
    }


    i32 EnabledInstanceLayerCount = 0;
    const char * AllInstanceLayers[] = {
        "VK_LAYER_KHRONOS_validation"
    };
    b32 InstanceLayersEnabledStatus[ArrayCount(AllInstanceLayers)] = {
        EnableValidationLayer
    };

    char * InstanceLayersToEnable[ArrayCount(AllInstanceLayers)];

    u32 InstanceLayerCount;
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,0));

    VkLayerProperties LayerProperties[40];
    Assert(ArrayCount(LayerProperties) >= InstanceLayerCount);
    VK_CHECK(vkEnumerateInstanceLayerProperties(&InstanceLayerCount,&LayerProperties[0]));

    for (u32 LayerIndex = 0;
                LayerIndex < ArrayCount(InstanceLayersEnabledStatus);
                ++LayerIndex)
    {
        if ( InstanceLayersEnabledStatus[LayerIndex] )
        {
            b32 Found  = false;
            for (u32 LayerPropIndex = 0;
                        LayerPropIndex < InstanceLayerCount;
                        ++LayerPropIndex)
            {
                Logn("%s",LayerProperties[LayerPropIndex].layerName);
                if (strcmp(LayerProperties[LayerPropIndex].layerName,AllInstanceLayers[LayerIndex]) == 0)
                {
                    InstanceLayersToEnable[EnabledInstanceLayerCount++] = (char *)AllInstanceLayers[LayerIndex];
                    Found = true;
                    break;
                }
            }
            if (!Found)
            {
                Log("Requested Instance Layer %s but it was not found\n.",AllInstanceLayers[LayerIndex]);
                return 1;
            }
        }
    }

    VkInstanceCreateInfo InstanceCreateInfo;
    InstanceCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // VkStructureType   sType;
    InstanceCreateInfo.pNext                   = 0;                                      // Void * pNext;
    InstanceCreateInfo.flags                   = 0;                                      // VkInstanceCreateFlags   flags;
    InstanceCreateInfo.pApplicationInfo        = &AppInfo;                               // Typedef * pApplicationInfo;
    InstanceCreateInfo.enabledLayerCount       = EnabledInstanceLayerCount;              // u32_t   enabledLayerCount;
    
    if (EnabledInstanceLayerCount)
    {
        InstanceCreateInfo.ppEnabledLayerNames = &InstanceLayersToEnable[0];             // Pointer * ppEnabledLayerNames;
    }
    InstanceCreateInfo.enabledExtensionCount   = ArrayCount(ExtensionsRequired);         // u32_t   enabledExtensionCount;
    InstanceCreateInfo.ppEnabledExtensionNames = &ExtensionsRequired[0];                 // Pointer * ppEnabledExtensionNames;

    VK_CHECK(vkCreateInstance(&InstanceCreateInfo,0,&GlobalVulkan.Instance));

    VkInstance Instance = GlobalVulkan.Instance;

    // Set pointer to OS dependent function to create surface
    // OS .exe is responsible to cast this pointer and use it
    *pfnOSSurface = vkGetInstanceProcAddr(Instance,OSSurfaceFuncName);

    // Now we can get address of instance functions
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroySurfaceKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceSupportKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfaceFormatsKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceSurfacePresentModesKHR);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceMemoryProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFormatProperties);
    

    // physical devices
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumeratePhysicalDevices);
    VK_INSTANCE_LEVEL_FN(Instance,vkEnumerateDeviceExtensionProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFeatures);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceFeatures2);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetPhysicalDeviceQueueFamilyProperties);
    VK_INSTANCE_LEVEL_FN(Instance,vkCreateDevice);
    VK_INSTANCE_LEVEL_FN(Instance,vkGetDeviceProcAddr);
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroyInstance);
    VK_INSTANCE_LEVEL_FN(Instance,vkCreateCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkDestroyCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkResetCommandPool);
    VK_INSTANCE_LEVEL_FN(Instance,vkResetCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkAllocateCommandBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkFreeCommandBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkBeginCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkEndCommandBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdCopyBuffer);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdCopyBufferToImage);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdPushConstants);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBeginRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdEndRenderPass);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdPipelineBarrier);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdClearColorImage);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindPipeline);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdDraw);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdDrawIndexed);
    VK_INSTANCE_LEVEL_FN(Instance,vkQueueSubmit);
    VK_INSTANCE_LEVEL_FN(Instance,vkQueueWaitIdle);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindVertexBuffers);
    VK_INSTANCE_LEVEL_FN(Instance,vkCmdBindIndexBuffer);

    return 0;
}

struct vulkan_node
{
    void * VulkanObj;
    vulkan_node * Parent;
    vulkan_node * LeftMostChild; 
    vulkan_node * RightSibling;
};

struct vulkan_tree
{
    memory_arena Arena;
    vulkan_node * Root;
};

vulkan_tree *
CreateVulkanTree(memory_arena * Arena)
{
    u32 SizeArena = Megabytes(2);
    u32 SizeRequirements = sizeof(vulkan_tree) + SizeArena;

    vulkan_tree * Tree = (vulkan_tree *)PushSize(Arena, SizeRequirements);

    u8 * StartAddrArena = (u8 *)(Tree + sizeof(vulkan_tree));
    InitializeArena(&Tree->Arena, StartAddrArena, SizeArena);
}

struct queue_item
{
    vulkan_node ** Node;
    queue_item * next;
};

struct queue
{
    u32 Capacity;
    u32 Free;
    queue_item * First;
    queue_item * Last;
    queue_item * Items;
};

void
PushQueue(memory_arena * Arena, queue * Q, vulkan_node ** Node)
{
    if (Q->Free == 0)
    {
        PushSize(Arena, sizeof(queue_item) * Q->Capacity);
        Q->Free = Q->Capacity;
        Q->Capacity *= 2;
    }

    queue_item * Item = 0;
    queue_item * Last = Q->Last;

    for (int i = 0; i < Q->Capacity; ++i)
    {
        Item = Q->Items + i;
        if (!Item->Node)
        {
            break;
        }
    }

    Item->next = 0;
    Item->Node = Node;
    Last->next = Item;

    Q->Free -= 1;

}

vulkan_node *
VisitLevel(vulkan_tree * Tree, vulkan_node * Node, void * FindThis, queue * Q)
{
    if (!Node) return 0;
    while (Node)
    {
        Assert(Node->VulkanObj);
        if (Node->VulkanObj == FindThis)
        {
            return Node;
        }

        PushQueue(&Tree->Arena, Q, &Node);
        Node = Node->RightSibling;
    }
}


vulkan_node *
PopQueue(queue * Q)
{
    vulkan_node * Node = 0;
    queue_item * Item = Q->First;
    queue_item * Next = 0;

    if (Item) 
    {
        Next = Item->next;
        Node = *Item->Node;
    }

    Q->First = Next;
    Item->next = 0;
    Item->Node = 0;

    return Node;
}

b32
VulkanAddObject(vulkan_tree * Tree, void * ParentObj, void * VulkanObj)
{
    vulkan_node * Node = PushStruct(&Tree->Arena, vulkan_node);
    u32 ArenaBeginSize = Tree->Arena.CurrentSize;
    queue Q;
    Q.Capacity = 20;
    Q.Free = Q.Capacity - 1;
    Q.Items = (queue_item *)PushSize(&Tree->Arena, sizeof(queue_item) * Q.Capacity);
    Q.First = Q.Items;
    Q.Last = Q.Items + 1;
    Q.First->next = Q.Items + 1;
    Q.First->Node = &Tree->Root;
        

    if (!ParentObj)
    {
        Tree->Root = Node;
        Node->Parent = 0;
        Node->RightSibling = 0;
        Node->LeftMostChild = 0;
    }
    else
    {
        vulkan_node * ParentNode = PopQueue(&Q);
        while (ParentNode)
        {
            ParentNode = VisitLevel(Tree, ParentNode, VulkanObj, &Q);
            if (ParentNode)
            {
                break;
            }
            ParentNode = PopQueue(&Q);
        }
    }

    Tree->Arena.CurrentSize = ArenaBeginSize;

    return 1;
}


i32
InitializeVulkan(i32 Width, i32 Height, 
                 graphics_platform_window PlatformWindow,
                 b32 EnableValidationLayer,
                 PFN_vkGetInstanceProcAddr GetInstanceProcAddr)
{
    // during creation of vulkan instance
    // base on the OS surface extension name
    // get a void pointer to function which
    // we give back to the OS to cast and invoke
    void * pfnOSSurface = 0;

    if (VulkanGetInstance(EnableValidationLayer,
                          PlatformWindow.VkKHROSSurfaceExtensionName,
                          &pfnOSSurface,PlatformWindow.OSSurfaceFuncName,
                          GetInstanceProcAddr)) return 1;
    QUEUE_DELETE_INSTANCE(&PrimaryVulkanDestroyQueue,&GlobalVulkan.Instance);

#if DEBUG
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkCreateDebugUtilsMessengerEXT);
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkDestroyDebugUtilsMessengerEXT);
    VK_INSTANCE_LEVEL_FN(GlobalVulkan.Instance,vkSetDebugUtilsObjectNameEXT);

    VkDebugUtilsMessengerCreateInfoEXT DebugUtilMsgCreateInfo;
    
    DebugUtilMsgCreateInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT; // sType   sType
    DebugUtilMsgCreateInfo.pNext           = 0; // Void * pNext
    DebugUtilMsgCreateInfo.flags           = 0; // flags   flags
    DebugUtilMsgCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT; // messageSeverity   messageSeverity
    DebugUtilMsgCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT  | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT; // messageType   messageType
    DebugUtilMsgCreateInfo.pfnUserCallback = VulkanDebugMessageCallback; // pfnUserCallback   pfnUserCallback
    DebugUtilMsgCreateInfo.pUserData       = 0; // Void * pUserData

    // you can have multiple callbacks for different messages
    vkCreateDebugUtilsMessengerEXT(GlobalVulkan.Instance , &DebugUtilMsgCreateInfo, 0, &GlobalVulkan.DefaultDebugCb);

    QUEUE_DELETE_DEBUG_MSG(&PrimaryVulkanDestroyQueue,GlobalVulkan.Instance,&GlobalVulkan.DefaultDebugCb);
#endif

    if (PlatformWindow.pfnVulkanCreateSurface(PlatformWindow.SurfaceData,pfnOSSurface, GlobalVulkan.Instance, &GlobalVulkan.Surface)) return 1;
    QUEUE_DELETE_SURFACEKHR(&PrimaryVulkanDestroyQueue,GlobalVulkan.Instance,&GlobalVulkan.Surface);

    if (VulkanGetPhysicalDevice()) return 1;
    if (VulkanCreateLogicaDevice()) return 1;
    QUEUE_DELETE_DEVICE(&PrimaryVulkanDestroyQueue,&GlobalVulkan.PrimaryDevice);

    if (VulkanCreateSwapChain(Width, Height)) return 1;
    QUEUE_DELETE_SWAPCHAINKHR(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan.Swapchain);

    CreateUnallocatedMemoryArenas(GlobalVulkan.MemoryArenas,
                                  ArrayCount(GlobalVulkan.MemoryArenas),
                                  &GlobalVulkan.MemoryArenaCount);

    for (u32 i = 0; i < GlobalVulkan.MemoryArenaCount;++i)
    {
        gpu_arena * Arena = GlobalVulkan.MemoryArenas + i;
        QUEUE_DELETE_ARENA(&PrimaryVulkanDestroyQueue,Arena);
    }

    CommitArenasToMemory(&GlobalVulkan.MemoryArenas[0],
                         GlobalVulkan.MemoryArenaCount,
                         &GlobalVulkan.DeviceMemoryPools);

    /* INIT DESCRIPTOR SETS */
    /* Level 1 */ CreateDescriptorSetLayoutGlobal();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._GlobalSetLayout);
    SetDebugName(GlobalVulkan._GlobalSetLayout,"_GlobalSetLayout");
    /* Level 2 */ CreateDescriptorSetLayoutObjects();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._ObjectsSetLayout);
    SetDebugName(GlobalVulkan._ObjectsSetLayout,"_ObjectsSetLayout");
    /* Level 3 */ CreateDescriptorSetTextures();
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._DebugTextureSetLayout);
    SetDebugName(GlobalVulkan._DebugTextureSetLayout,"_DebugTextureSetLayout");
#if 1
    /* Level 4 */ CreateDescriptorSetAttachmentInputs(&GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout);
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout);
    SetDebugName(GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout,"_oit_WeightedColorAttachmentInputsSetLayout");

    /* Level 5 */ CreateDescriptorSetAttachmentInputs(&GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout);
    QUEUE_DELETE_DESCRIPTORSETLAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout);
    SetDebugName(GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout,"_oit_WeightedRevealAttachmentInputsSetLayout");
#endif

    VkDeviceSize TextureBufferSize = Megabytes(15);
    VkDescriptorPoolSize DescriptorPoolSizes[] =
	{
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 2 }
	};

	VkDescriptorPoolCreateInfo PoolInfo = {};
	PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	PoolInfo.flags = 0;
	PoolInfo.maxSets = 10;
	PoolInfo.poolSizeCount = ArrayCount(DescriptorPoolSizes);
	PoolInfo.pPoolSizes = DescriptorPoolSizes;

	vkCreateDescriptorPool(GlobalVulkan.PrimaryDevice, &PoolInfo, nullptr, &GlobalVulkan._DescriptorPool);
    SetDebugName(GlobalVulkan._DescriptorPool,"_DescriptorPool");
    QUEUE_DELETE_DESCRIPTORPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan._DescriptorPool);

    // (fef) for each frame
    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;

        VkFenceCreateInfo FenceCreateInfo;

        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO; // VkStructureType   sType;
        FenceCreateInfo.pNext = 0;                                   // Void * pNext;
        FenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;        // VkFenceCreateFlags   flags;

        VK_CHECK(vkCreateFence(GlobalVulkan.PrimaryDevice, &FenceCreateInfo, 0,&FrameData->RenderFence));

        VkSemaphoreCreateInfo SemaphoreCreateInfo = {
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            0,
            0
        };

        VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &FrameData->ImageAvailableSemaphore));
        VK_CHECK(vkCreateSemaphore(GlobalVulkan.PrimaryDevice, &SemaphoreCreateInfo, 0, &FrameData->RenderSemaphore));

        if (VH_CreateCommandPool(
                    GlobalVulkan.PrimaryDevice,
                    GlobalVulkan.PresentationQueueFamilyIndex,
                    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                    &FrameData->CommandPool
                    )) return 1;

        if (VH_CreateCommandBuffers(
                    GlobalVulkan.PrimaryDevice,
                    FrameData->CommandPool,
                    1,
                    &FrameData->PrimaryCommandBuffer)) return 1;

        // from pool, this layout, out ID
        VH_AllocateDescriptor(&GlobalVulkan._GlobalSetLayout, GlobalVulkan._DescriptorPool, &FrameData->GlobalDescriptor);

        VkDescriptorBufferInfo BufferInfo;
        BufferInfo.buffer = GlobalVulkan.SimulationArena->Buffer; // VkBuffer   buffer;
        BufferInfo.offset = 0;
        BufferInfo.range  = sizeof(GPUSimulationData); // VkDeviceSize   range;

        VkWriteDescriptorSet SimulationWriteSet =
            VH_WriteDescriptor(0,FrameData->GlobalDescriptor, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, &BufferInfo);

        VH_AllocateDescriptor(&GlobalVulkan._ObjectsSetLayout, GlobalVulkan._DescriptorPool, &FrameData->ObjectsDescriptor);

        VkDescriptorBufferInfo BufferInfoObjects;
        BufferInfoObjects.buffer = FrameData->ObjectsArena->Buffer; // VkBuffer   buffer;
        BufferInfoObjects.offset = 0;
        BufferInfoObjects.range  = FrameData->ObjectsArena->MaxSize; // VkDeviceSize   range;

        VkWriteDescriptorSet ObjectsWriteSet =
            VH_WriteDescriptor(0,FrameData->ObjectsDescriptor, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, &BufferInfoObjects);

        VkWriteDescriptorSet WriteSets[] = 
        {
            SimulationWriteSet,
            ObjectsWriteSet
        };

        vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);
    }

    for (i32 FrameIndex = 0;
            FrameIndex < FRAME_OVERLAP;
            ++FrameIndex)
    {
        frame_data * FrameData = GlobalVulkan.FrameData + FrameIndex;
        QUEUE_DELETE_COMMANDPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&FrameData->CommandPool);
        QUEUE_DELETE_FENCE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->RenderFence);
        QUEUE_DELETE_SEMAPHORE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->ImageAvailableSemaphore);
        QUEUE_DELETE_SEMAPHORE(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &FrameData->RenderSemaphore);
    }

    // INIT PIPELINES
    CreatePipelineLayoutTexture(GlobalVulkan.PipelineLayout + 0);
    SetDebugName(GlobalVulkan.PipelineLayout[0], "PipelineLayout Texture");
    QUEUE_DELETE_PIPELINELAYOUT(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.PipelineLayout[0]);

    // Staging buffer commands
    if (VH_CreateCommandPool(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.TransferOnlyQueueFamilyIndex,
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                &GlobalVulkan.CommandPoolTransferBit
                )) return 1;

    QUEUE_DELETE_COMMANDPOOL(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice,&GlobalVulkan.CommandPoolTransferBit);

    if (VH_CreateCommandBuffers(
                GlobalVulkan.PrimaryDevice,
                GlobalVulkan.CommandPoolTransferBit,
                1,
                &GlobalVulkan.TransferBitCommandBuffer)) return 1;


    VkExtent3D Extent3D = { GlobalVulkan.WindowExtension.width, GlobalVulkan.WindowExtension.height, 1};
    GlobalVulkan.PrimaryDepthBuffer = VH_CreateDepthBuffer(GlobalVulkan.PrimaryDepthBufferArena,Extent3D);
    if (IS_NULL(GlobalVulkan.PrimaryDepthBuffer))
    {
        Logn("Failed to create depth buffer");
        return 1;
    }
    SetDebugName(GlobalVulkan.PrimaryDepthBuffer->Image, "PrimaryDepthBuffer");

    VkImageUsageFlags WeightUsageFlags = 
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    GlobalVulkan.WeightedColorImage = VH_CreateImage(GlobalVulkan.WeightedColorArena, 
                                                     VK_FORMAT_R16G16B16A16_SFLOAT, 
                                                     WeightUsageFlags, 
                                                     Extent3D);
    if (IS_NULL(GlobalVulkan.WeightedColorImage))
    {
        Logn("Failed to create weighted color image");
    }
    SetDebugName(GlobalVulkan.WeightedColorImage->Image, "WeightedColorImage");


    GlobalVulkan.WeightedRevealImage = VH_CreateImage(GlobalVulkan.WeightedRevealArena, 
                                                     VK_FORMAT_R16_SFLOAT, 
                                                     WeightUsageFlags, 
                                                     Extent3D);
    if (IS_NULL(GlobalVulkan.WeightedRevealImage))
    {
        Logn("Failed to create weighted reveal image");
    }
    SetDebugName(GlobalVulkan.WeightedRevealImage->Image, "WeightedRevealImage");

    VkImageSubresourceRange Range;
    Range.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    Range.baseMipLevel   = 0; // uint32_t baseMipLevel;
    Range.levelCount     = 1; // uint32_t levelCount;
    Range.baseArrayLayer = 0; // uint32_t baseArrayLayer;
    Range.layerCount     = 1; // uint32_t layerCount;

    VkImageMemoryBarrier MemoryBarrierSetColorAttachment;
    MemoryBarrierSetColorAttachment.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER; // sType   sType
    MemoryBarrierSetColorAttachment.pNext               = 0; // Void * pNext
    MemoryBarrierSetColorAttachment.srcAccessMask       = VK_ACCESS_NONE; // srcAccessMask   srcAccessMask
    MemoryBarrierSetColorAttachment.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // dstAccessMask   dstAccessMask
    MemoryBarrierSetColorAttachment.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED; // oldLayout   oldLayout
    MemoryBarrierSetColorAttachment.newLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // newLayout   newLayout
    MemoryBarrierSetColorAttachment.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // srcQueueFamilyIndex   srcQueueFamilyIndex
    MemoryBarrierSetColorAttachment.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // dstQueueFamilyIndex   dstQueueFamilyIndex
    MemoryBarrierSetColorAttachment.subresourceRange    = Range; // subresourceRange   subresourceRange

    VkCommandBuffer SingleCmd = BeginSingleCommandBuffer();

    MemoryBarrierSetColorAttachment.image               = GlobalVulkan.WeightedRevealImage->Image; // image   image
    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &MemoryBarrierSetColorAttachment);

    MemoryBarrierSetColorAttachment.image               = GlobalVulkan.WeightedColorImage->Image; // image   image
    vkCmdPipelineBarrier(SingleCmd, 
                            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 
                            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &MemoryBarrierSetColorAttachment);

    EndSingleCommandBuffer(SingleCmd);

    if (VulkanInitDefaultRenderPass()) return 1;
    QUEUE_DELETE_RENDERPASS(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.RenderPass);
    if (VulkanCreateTransparentRenderPass()) return 1;
    QUEUE_DELETE_RENDERPASS(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.RenderPassTransparency);

    if (VulkanInitFramebuffers()) return 1;
    for (u32 ImageIndex = 0;
                ImageIndex < GlobalVulkan.SwapchainImageCount;
                ++ImageIndex)
    {
        QUEUE_DELETE_FRAMEBUFFER(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.Framebuffers[ImageIndex]);
        QUEUE_DELETE_FRAMEBUFFER(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.FramebuffersTransparency[ImageIndex]);
        QUEUE_DELETE_IMAGEVIEW(&PrimaryVulkanDestroyQueue,GlobalVulkan.PrimaryDevice, &GlobalVulkan.SwapchainImageViews[ImageIndex]);
    }

    // As for now a single sampler for all textures
    VkSamplerCreateInfo SamplerCreateInfo = VulkanSamplerCreateInfo(VK_FILTER_LINEAR,VK_SAMPLER_ADDRESS_MODE_REPEAT);

    VK_CHECK(vkCreateSampler(GlobalVulkan.PrimaryDevice,&SamplerCreateInfo,0,&GlobalVulkan.TextureSampler));

    // TODO: how to properly handle dummy texture on initialization?
    i32 DummyImgData = (0xFF << 24) |
                       (0xFF << 0 );

    PushTextureData(&DummyImgData, 1, 1, 4);

    // Update descriptors once oit weight images are created
    {
        VH_AllocateDescriptor(&GlobalVulkan._oit_WeightedColorAttachmentInputsSetLayout, GlobalVulkan._DescriptorPool, &GlobalVulkan._oit_WeightedColorSet);
        VH_AllocateDescriptor(&GlobalVulkan._oit_WeightedRevealAttachmentInputsSetLayout, GlobalVulkan._DescriptorPool, &GlobalVulkan._oit_WeightedRevealSet);


        VkDescriptorImageInfo OitWeightedColorImageInfo;
        OitWeightedColorImageInfo.sampler     = VK_NULL_HANDLE; // sampler   sampler
        OitWeightedColorImageInfo.imageView   = GlobalVulkan.WeightedColorImage->ImageView; // imageView   imageView
        OitWeightedColorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // imageLayout   imageLayout

        VkWriteDescriptorSet OitWeightedColorWriteSet =
            VH_WriteDescriptor(0,GlobalVulkan._oit_WeightedColorSet, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &OitWeightedColorImageInfo);

        VkDescriptorImageInfo OitWeightedRevealImageInfo;
        OitWeightedRevealImageInfo.sampler     = VK_NULL_HANDLE; // sampler   sampler
        OitWeightedRevealImageInfo.imageView   = GlobalVulkan.WeightedRevealImage->ImageView; // imageView   imageView
        OitWeightedRevealImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // imageLayout   imageLayout

        VkWriteDescriptorSet OitWeightedRevealWriteSet =
            VH_WriteDescriptor(0,GlobalVulkan._oit_WeightedRevealSet, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, &OitWeightedRevealImageInfo);

        VkWriteDescriptorSet WriteSets[] = 
        {
            OitWeightedColorWriteSet,
            OitWeightedRevealWriteSet
        };

        vkUpdateDescriptorSets(GlobalVulkan.PrimaryDevice, ArrayCount(WriteSets), WriteSets, 0, nullptr);
    }

    GlobalVulkan.Initialized = true;

    return 0;
}
