#pragma once
#include <vulkan/vulkan_raii.hpp>
namespace lte {
	struct singleTimeCommandInfo {
		vk::raii::Device* device = nullptr;
		vk::CommandPool* CommandPool = nullptr;
		vk::raii::Queue* queue = nullptr;
		singleTimeCommandInfo(vk::raii::Device* Idevice, vk::CommandPool* IcommandPool, vk::raii::Queue* Iqueue) : device{ Idevice }, CommandPool{ IcommandPool }, queue{ Iqueue } {

		}
	};
	class CommandBuffers
	{
		
	public:
		static	void createCommandPool(vk::raii::CommandPool* commandPool, vk::raii::Device* device, uint32_t queueIndex);
		static std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands(vk::raii::Device* device, vk::CommandPool* commandPool);
		static void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer, vk::raii::Queue* queue);
		static void createCommandBuffer(std::vector<vk::raii::CommandBuffer>* commandBuffers, vk::raii::CommandPool* commandPool, vk::raii::Device* device, uint8_t maxFIF);
	};
}

