#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "SwapchainHandler.h"
namespace lte {
	struct LtSyncSet {
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores{};
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores{};
		std::vector<vk::raii::Fence> inFlightFences{};
	};
	class LtSync
	{
	public:
		static void createSyncObjects(LtSyncSet* set, LtSwapChain* swapchain, vk::raii::Device* device);
	};
}
