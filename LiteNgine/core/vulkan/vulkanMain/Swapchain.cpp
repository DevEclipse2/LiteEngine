#include "../VulkanDevice.h"
namespace lte {


	void VulkanDevice::createSwapChain()
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
		swapChainExtent = chooseSwapExtent(surfaceCapabilities);
		uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
		std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
		swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

		std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
		vk::PresentModeKHR              presentMode = chooseSwapPresentMode(availablePresentModes);

		vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
			swapChainCreateInfo.surface				= *surface,
			swapChainCreateInfo.minImageCount		= minImageCount,
			swapChainCreateInfo.imageFormat			= swapChainSurfaceFormat.format,
			swapChainCreateInfo.imageColorSpace		= swapChainSurfaceFormat.colorSpace,
			swapChainCreateInfo.imageExtent			= swapChainExtent,
			swapChainCreateInfo.imageArrayLayers	= 1,
			swapChainCreateInfo.imageUsage			= vk::ImageUsageFlagBits::eColorAttachment,
			swapChainCreateInfo.imageSharingMode	= vk::SharingMode::eExclusive,
			swapChainCreateInfo.preTransform		= surfaceCapabilities.currentTransform,
			swapChainCreateInfo.compositeAlpha		= vk::CompositeAlphaFlagBitsKHR::eOpaque,
			swapChainCreateInfo.presentMode			= chooseSwapPresentMode(availablePresentModes),
			swapChainCreateInfo.clipped				= true;
			swapChainCreateInfo.oldSwapchain		= nullptr;
		swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
		swapChainImages = swapChain.getImages();

	}
}