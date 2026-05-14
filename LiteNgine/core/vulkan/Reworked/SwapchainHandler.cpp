#include "SwapchainHandler.h"
namespace lte {
	
	vk::SurfaceFormatKHR SwapchainHandler::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
	{
		const auto formatIt = std::ranges::find_if(
			availableFormats,
			[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
		return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	}
	vk::PresentModeKHR SwapchainHandler::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
	{
		assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
		return std::ranges::any_of(availablePresentModes,
			[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
			vk::PresentModeKHR::eMailbox :
			vk::PresentModeKHR::eFifo;
	}
	uint32_t SwapchainHandler::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities,uint32_t* minImageCount)
	{
		auto minImageC = (std::max)(3u, surfaceCapabilities.minImageCount);
		if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < *minImageCount))
		{
			minImageC = surfaceCapabilities.maxImageCount;
		}
		*minImageCount = minImageC;
		return minImageC;
	}
	vk::Extent2D SwapchainHandler::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities, Lt_MultiWindow& window)
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


	void SwapchainHandler::cleanupSwapChain(LtSwapChain* swap)
	{
		swap->imageViews.clear();
		swap->swapChain = nullptr;
	}
	

}