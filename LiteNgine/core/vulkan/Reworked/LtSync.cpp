#include "LtSync.h"
namespace lte {

	void LtSync::createSyncObjects(LtSyncSet* set, LtSwapChain* swapchain , vk::raii::Device* device)
	{


		assert(set->presentCompleteSemaphores.empty() && set->renderFinishedSemaphores.empty() && set->inFlightFences.empty());

		for (size_t i = 0; i < swapchain->swapChainImages.size(); i++)
		{
			set->renderFinishedSemaphores.emplace_back(*device, vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{	
			vk::FenceCreateInfo info{};
			info.flags = vk::FenceCreateFlagBits::eSignaled;
			set->presentCompleteSemaphores.emplace_back(*device, vk::SemaphoreCreateInfo());
			set->inFlightFences.emplace_back(device, info);
		}
	}
}