#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "DeviceHandler.h"
#include "CommandBuffers.h"
#include "LtMesh.h"
namespace lte {
	class Buffers
	{
	public:
		static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory, vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice);
		static void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height, singleTimeCommandInfo info);
		static void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size, singleTimeCommandInfo info);	
		static void createVertexBuffer	(uint32_t size, Vertex* vertex	, vk::raii::Buffer* buffer, vk::raii::DeviceMemory* deviceMemory, singleTimeCommandInfo info, vk::raii::PhysicalDevice& device);
		static void createIndexBuffer	(uint32_t size, uint32_t* vertex, vk::raii::Buffer* buffer, vk::raii::DeviceMemory* deviceMemory, singleTimeCommandInfo info, vk::raii::PhysicalDevice& device);
		static void createUniformBuffers(std::vector<LtMeshInfo>* meshes, uint8_t maxFIF, vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice);



	};
}

