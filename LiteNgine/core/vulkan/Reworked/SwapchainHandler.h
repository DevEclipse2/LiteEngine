#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "../Lt_Window.h"
#include "../EngineClasses/Lt_MultiWindow.h"
namespace lte
{
	struct LtSwapChain;
	class SwapchainHandler
	{
	public:
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
		static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
		static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities, uint32_t* minImageCount);
		static void cleanupSwapChain(LtSwapChain* swap);
		static vk::Extent2D chooseSwapExtentOld(vk::SurfaceCapabilitiesKHR const& capabilities, Lt_Window* window);
		static vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities, Lt_MultiWindow& window);

	};

	struct LtSwapChain {
		vk::Extent2D swapChainExtent;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages = {};
		std::vector<vk::raii::ImageView> imageViews = {};

		uint32_t colorImage = 0;
		uint32_t depthImage = 0;

		LtSwapChain(vk::raii::PhysicalDevice& physicalDevice, vk::raii::Device& device, vk::raii::SurfaceKHR& surface, Lt_MultiWindow& window, uint32_t* minimgC)
		{

			vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
			swapChainExtent = SwapchainHandler::chooseSwapExtent(surfaceCapabilities, window);
			uint32_t minImageCount = SwapchainHandler::chooseSwapMinImageCount(surfaceCapabilities, minimgC);
			std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
			swapChainSurfaceFormat = SwapchainHandler::chooseSwapSurfaceFormat(availableFormats);

			std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
			vk::PresentModeKHR              presentMode = SwapchainHandler::chooseSwapPresentMode(availablePresentModes);


			if (!(surfaceCapabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eSampled)) {
				// You cannot use eSampled on this swapchain!
			}


			vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
			swapChainCreateInfo.surface = *surface,
				swapChainCreateInfo.minImageCount = minImageCount,
				swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format,
				swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace,
				swapChainCreateInfo.imageExtent = swapChainExtent,
				swapChainCreateInfo.imageArrayLayers = 1,
				swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
				swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive,
				swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform,
				swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
				swapChainCreateInfo.presentMode = SwapchainHandler::chooseSwapPresentMode(availablePresentModes),
				swapChainCreateInfo.clipped = true;
			swapChainCreateInfo.oldSwapchain = nullptr;
			swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
			swapChainImages = swapChain.getImages();
		}

	};
}

