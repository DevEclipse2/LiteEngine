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
#include "LtMesh.h"
#include "ImageDelegate.h"
namespace lte {
	struct LogicalDevice {
		vk::raii::Device device = nullptr;
		vk::raii::Queue queue = nullptr;
		uint32_t queueIndex = 0;
	};

	class DeviceHandler
	{
		public:

			static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice& physicalDevice);

			void pickPhysicalDevice(vk::raii::Instance& instance, vk::raii::PhysicalDevice& physicalDevice, vk::SampleCountFlagBits& sampling);
			vk::SampleCountFlagBits getMaxUsableSampleCount(vk::raii::PhysicalDevice& physicalDevice);
			void createLogicalDevice(vk::raii::PhysicalDevice& physicalDevice, vk::raii::SurfaceKHR& surface, LogicalDevice& logicalDevice, std::vector<const char*> requiredExtensions);
			void createTextureSampler(vk::raii::Sampler* sampler, vk::raii::PhysicalDevice& physicalDevice, vk::raii::Device& device);
			void createDescriptorPool(vk::raii::DescriptorPool* descriptorPool, vk::raii::Device* device, uint32_t maxObjects, uint8_t maxFIF);
			void createDescriptorSets(vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::DescriptorPool& descriptorPool, vk::raii::Sampler& sampler, std::vector<LtMeshInfo>& meshes, uint8_t maxFIF, vk::raii::Device& device, std::vector<RenderSet>& rs);

	};
}
