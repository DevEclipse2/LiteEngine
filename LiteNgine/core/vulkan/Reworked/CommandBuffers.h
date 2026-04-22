#pragma once
#include <vulkan/vulkan_raii.hpp>
namespace lte {
	class CommandBuffers
	{
	public:
		static std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands(vk::raii::Device* device, vk::CommandPool* commandPool);
		static void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue* queue);

	};
}

