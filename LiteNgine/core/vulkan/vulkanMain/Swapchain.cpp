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
			swapChainCreateInfo.imageUsage			= vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled| vk::ImageUsageFlagBits::eTransferSrc,
			swapChainCreateInfo.imageSharingMode	= vk::SharingMode::eExclusive,
			swapChainCreateInfo.preTransform		= surfaceCapabilities.currentTransform,
			swapChainCreateInfo.compositeAlpha		= vk::CompositeAlphaFlagBitsKHR::eOpaque,
			swapChainCreateInfo.presentMode			= chooseSwapPresentMode(availablePresentModes),
			swapChainCreateInfo.clipped				= true;
			swapChainCreateInfo.oldSwapchain		= nullptr;
		swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
		swapChainImages = swapChain.getImages();

	}
	vk::SurfaceFormatKHR VulkanDevice::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
	{
		const auto formatIt = std::ranges::find_if(
			availableFormats,
			[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;});
		return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	}
	vk::PresentModeKHR VulkanDevice::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
	{
		assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
		return std::ranges::any_of(availablePresentModes,
			[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
			vk::PresentModeKHR::eMailbox :
			vk::PresentModeKHR::eFifo;
	}
	uint32_t VulkanDevice::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
	{
		auto minImageC = (std::max)(3u, surfaceCapabilities.minImageCount);
		if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
		{
			minImageC = surfaceCapabilities.maxImageCount;
		}
		minImageCount = minImageC;
		return minImageC;
		


	}
	vk::Extent2D VulkanDevice::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		int w, h;
		glfwGetFramebufferSize(window.getGLFWWindow(), &w, &h);
		uint32_t width = static_cast<uint32_t>(w);
		uint32_t height = static_cast<uint32_t>(h);
		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}
}