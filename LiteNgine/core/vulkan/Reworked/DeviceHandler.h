/*
* what is this? 
*this is a class to handle most things related to device that don't really fall under a specific category / are too general use
* 
*/

#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <map>
namespace lte {
	struct LogicalDevice {
		vk::raii::Device device = nullptr;
		vk::raii::Queue queue = nullptr;
		uint32_t queueIndex = 0;
	};

	class DeviceHandler
	{
		public:
			static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice physicalDevice);
			void pickPhysicalDevice(vk::raii::Instance* instance, vk::raii::PhysicalDevice* physicalDevice, vk::SampleCountFlagBits* sampling);
			vk::SampleCountFlagBits getMaxUsableSampleCount(vk::raii::PhysicalDevice* physicalDevice);
			void createLogicalDevice(vk::raii::PhysicalDevice* physicalDevice, vk::raii::SurfaceKHR* surface, LogicalDevice* logicalDevice, std::vector<const char*> requiredExtensions);

	};
}
