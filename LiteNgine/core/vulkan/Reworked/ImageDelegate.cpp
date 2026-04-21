#include "ImageDelegate.h"
namespace lte {
	uint32_t ImageDelegate::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice physicalDevice)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}

}