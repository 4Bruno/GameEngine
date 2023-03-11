/*
 * - Initialize library pfn (AKA Instance)
 * - Create debug callback if errors
 * - Create surface
 * - Pick physical device
 * - Create Logical device for physical device
 * - Create swapchain & swapchain views
 * - Create all unalloc buffers (wrapped in arenas)
 *   This step is to understand memory req
 *
 * - Allocate device mem pools
 * - Define Descriptor set LAYOUTS
 * - Create descriptor pool
 * - Create frames command pools/barriers
 * - Create Pipeline Layouts
 * - Create staging command buffers
 * - Create depth buffer/color images
 * - Transition color images to present
 * - Create render passes
 * - Create frame buffers
 *   This is delayed as we need depth buffer created
 *   Otherwise should go with swap chain stuff
 *
 * - Create sampler for all textures
 * - Update descriptor set for OIT images
 *   Can be done earlier but for I do it 
 *   at the end to have clear code
 *
 *
 *
 *   FRAME data 
 *   buffers (arena) and  descriptor sets
 *   are depedant but is not handle in code
 *   if you destroy arenas (dont know the case yet)
 *   you will need to update frame data descriptors.
 *   Meaning bind to the new memory
 *
 */
#ifndef VULKAN_INITIALIZER_H
#define VULKAN_INITIALIZER_H


#define MAX_GPU_FRAME_ENTITIES 1000
struct vulkan_create_instance
{
    VkApplicationInfo AppInfo;
    VkInstanceCreateInfo CreateInfo;
    u32 NumberOfExtensions;
    char ** ExtensionsRequired;
    u32 NumberOfInstanceLayers;
    char ** InstanceLayers;
};

struct vulkan_node
{
    void * ID;
    void * Owner;
    vulkan_destructor_type DestructorType;
    void * Data;
};


#endif
