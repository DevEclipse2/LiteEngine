/*
* what is this? 
*this is a class to handle most things related to device that don't really fall under a specific category / are too general use
* 
*/

#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace lte {
	class DeviceHandler
	{
		public:
			static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice physicalDevice);
	};
}
