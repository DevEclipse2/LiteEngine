#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <map>
#include "Lt_Window.h"
#include "ShaderLoader.h"
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "VertexHandler.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace lte {
	class Lt_Window;
	class ShaderLoader;
	class VulkanDevice
	{
	public:
		#ifdef NDEBUG
			static constexpr bool enableValidationLayers = false;
		#else
			static constexpr bool enableValidationLayers = true;
		#endif
		VulkanDevice(Lt_Window* window);
		~VulkanDevice();
		const std::vector<char const*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
			};
		const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
		void drawFrame();
		void Exit();
		
		bool framebufferResized = false;

	private:

		std::vector<const char*> getRequiredInstanceExtensions();

		vk::raii::Context  context;

		vk::raii::Instance instance = nullptr;
		int createInstance();

		//debug messenger
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
			vk::DebugUtilsMessageTypeFlagsEXT              type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		void setupDebugMessenger();
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;

		//pick device
		void pickPhysicalDevice();
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
		void ListFeatures(vk::PhysicalDeviceProperties* props, vk::PhysicalDeviceFeatures* feats,vk::PhysicalDeviceMemoryProperties* memoryprops);
		vk::raii::Device device = nullptr;
		//logical device creation
		vk::raii::Queue queue				= nullptr;
		void createLogicalDevice();

		//windows surface recreation
		vk::raii::SurfaceKHR surface		= nullptr;
		void createSurface();
		Lt_Window& window;
		vk::SurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats);
		vk::PresentModeKHR chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes);
		vk::Extent2D	chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities);

		//swapchain creation
		void createSwapChain();
		vk::Extent2D swapChainExtent;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		void createImageViews();
		std::vector<vk::raii::ImageView> swapChainImageViews;


		//pipeline creation
		void createGraphicsPipeline();
		vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;
		static std::vector<char> readFile(const std::string& filename);


		//command pool 
		void createCommandPool();
		uint32_t    queueIndex = ~0;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		
		VertexHandler vertexHandler;
		void createVertexBuffer();
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);


		void createCommandBuffer();
		vk::raii::CommandPool    commandPool = nullptr;
		void recordCommandBuffer(uint32_t imageIndex);
		void transition_image_layout(
			uint32_t imageIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask
		);

		//sync objects
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores;
		std::vector<vk::raii::Fence> inFlightFences;
		void createSyncObjects();
		//multiple frame handlers
		uint32_t frameIndex = 0;

		//shader loading class
		ShaderLoader shaderLoader{};




		//swap chain recreation
		void recreateSwapChain();
		void cleanupSwapChain();
	};
}
