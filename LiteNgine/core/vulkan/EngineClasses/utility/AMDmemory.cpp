#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"


VmaAllocator allocator;

void InitVMA(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = logicalDevice;
    allocatorInfo.instance = instance;

    // If your engine uses Vulkan 1.2 or 1.3, specify it here to unlock advanced memory features
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

    vmaCreateAllocator(&allocatorInfo, &allocator);
}