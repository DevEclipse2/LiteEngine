#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "DeviceHandler.h"
#include "CommandBuffers.h"
namespace lte {
	class Buffers
	{
	public:
		static void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory, vk::raii::Device* device);
		static void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height, singleTimeCommandInfo info);

	};
}

