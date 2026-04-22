#pragma once
#include <vulkan/vulkan_raii.hpp>
#include "../Lt_Window.h"
namespace lte
{
	struct LtSwapChain {
		vk::Extent2D swapChainExtent;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages = {};
		LtSwapChain(vk::raii::PhysicalDevice* physicalDevice, vk::raii::Device* device, vk::raii::SurfaceKHR* surface, Lt_Window* window, uint32_t* minimgC)
		{
			if (physicalDevice == nullptr || device == nullptr || surface == nullptr || window == nullptr || minimgC == nullptr) return;
			vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice->getSurfaceCapabilitiesKHR(**surface);
			swapChainExtent = SwapchainHandler::chooseSwapExtent(surfaceCapabilities, window);
			uint32_t minImageCount = SwapchainHandler::chooseSwapMinImageCount(surfaceCapabilities, minimgC);
			std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice->getSurfaceFormatsKHR(**surface);
			swapChainSurfaceFormat = SwapchainHandler::chooseSwapSurfaceFormat(availableFormats);

			std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice->getSurfacePresentModesKHR(**surface);
			vk::PresentModeKHR              presentMode = SwapchainHandler::chooseSwapPresentMode(availablePresentModes);

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
				swapChainCreateInfo.presentMode = chooseSwapPresentMode(availablePresentModes),
				swapChainCreateInfo.clipped = true;
			swapChainCreateInfo.oldSwapchain = nullptr;
			swapChain = vk::raii::SwapchainKHR(*device, swapChainCreateInfo);
			swapChainImages = swapChain.getImages();

		}
	};

	class SwapchainHandler
	{
	public:
		static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
		static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
		static uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities, uint32_t* minImageCount);

		static vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities, Lt_Window* window);
		static vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);

	};
}

