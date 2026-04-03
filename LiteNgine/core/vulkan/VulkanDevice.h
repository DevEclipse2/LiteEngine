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
#include "testNcoolShit/TestAnim.h"
#include <chrono>
#include <stb_image.h>
#include "ConsoleLog.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>

constexpr int MAX_FRAMES_IN_FLIGHT = 3;

namespace lte {
	class Lt_Window;
	class ShaderLoader;
	class VulkanDevice
	{
	public:

		const std::string MODEL_PATH = "models/viking_room.obj";
		const std::string TEXTURE_PATH = "textures/viking_room.png";


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




		void createSwapChain();
		vk::Extent2D swapChainExtent;
		vk::SurfaceFormatKHR swapChainSurfaceFormat;
		uint32_t chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities);
		vk::raii::SwapchainKHR swapChain = nullptr;
		std::vector<vk::Image> swapChainImages;
		void createImageViews();
		std::vector<vk::raii::ImageView> swapChainImageViews;

		//shader description set 
		void createDescriptorSetLayout();
		

		//pipeline creation
		void createGraphicsPipeline();
		vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;
		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;
		vk::raii::Pipeline graphicsPipeline = nullptr;
		static std::vector<char> readFile(const std::string& filename);


		//command pool 
		void createCommandPool();
		uint32_t    queueIndex = ~0;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		

		void createVertexBuffer();
		//void updateVertexBuffer();
		void createIndexBuffer();
		void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		vk::raii::Buffer indexBuffer = nullptr;
		vk::raii::DeviceMemory indexBufferMemory = nullptr;
		void createUniformBuffers();
		void updateUniformBuffer(uint32_t frameindex);
		std::vector<vk::raii::Buffer> uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		void createDescriptorPool();

		vk::raii::DescriptorPool descriptorPool = nullptr;
		void createDescriptorSets();
		//vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		//test 
		TestAnim testAnim;
		int frameNumber;


		void createCommandBuffer();
		vk::raii::CommandPool    commandPool = nullptr;
		void recordCommandBuffer(uint32_t imageIndex);
		void transition_image_layout(
			vk::Image               image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask,
			vk::ImageAspectFlags    image_aspect_flags
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


		//texture stuff
		void createTextureImage();
		uint32_t mipLevels;
		vk::raii::Image        textureImage = nullptr;
		//std::unique_ptr<vk::raii::Image> textureImage;
		vk::raii::DeviceMemory textureImageMemory = nullptr;
		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory);
		std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
		void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);
		void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);
		void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);



		void createTextureImageView();
		vk::raii::ImageView createImageView(const vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
		void createTextureSampler();
		vk::raii::ImageView textureImageView	= nullptr;
		vk::raii::Sampler textureSampler		= nullptr;
		
		vk::raii::Image depthImage = nullptr;
		vk::raii::DeviceMemory depthImageMemory = nullptr;
		vk::raii::ImageView depthImageView = nullptr;
		void createDepthResources();
		vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format findDepthFormat();
		bool hasStencilComponent(vk::Format format);
		//vk::Format depthFormat = findDepthFormat();


		void loadModel();


		vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
		vk::SampleCountFlagBits getMaxUsableSampleCount();
		vk::raii::Image colorImage = nullptr;
		vk::raii::DeviceMemory colorImageMemory = nullptr;
		vk::raii::ImageView colorImageView = nullptr;
		void createColorResources();

	};
}
