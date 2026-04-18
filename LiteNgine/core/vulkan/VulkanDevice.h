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

#include "../engine/Gui.h"

#include <random>

constexpr int MAX_FRAMES_IN_FLIGHT = 3;
constexpr int MAX_OBJECTS = 3;

namespace lte {
	class Lt_Window;
	class ShaderLoader;
	class Gui;
	


	class VulkanDevice
	{
	public:

		struct meshObject {
			// Transform properties
			glm::vec3 position = { 0.0f, 0.0f, 0.0f };
			glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
			glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

			// Uniform buffer for this object (one per frame in flight)
			std::vector<vk::raii::Buffer> uniformBuffers;
			std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
			std::vector<void*> uniformBuffersMapped;

			// Descriptor sets for this object (one per frame in flight)
			std::vector<vk::raii::DescriptorSet> descriptorSets;

			// Calculate model matrix based on position, rotation, and scale
			glm::mat4 getModelMatrix() const {
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::translate(model, position);
				model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
				model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
				model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
				model = glm::scale(model, -scale);
				return model;
			}
		};

		struct Particle
		{
			glm::vec2 position;
			glm::vec2 velocity;
			glm::vec4 color;

			static vk::VertexInputBindingDescription getBindingDescription()
			{
				return { 0, sizeof(Particle), vk::VertexInputRate::eVertex };
			}

			static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
			{
				return {
					vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Particle, position)),
					vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Particle, color)),
				};
			}
		};

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
		void getFrameBufferSize(int* width, int* height);
		int WIDTH = 800;
		int HEIGHT = 600;
		
		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory);
		uint32_t minImageCount = 0;
		void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory);
		void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
		void copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height);
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
		std::vector<vk::raii::CommandBuffer>*	getCommandBuffer();
		vk::raii::Device*						getDevice();
		GLFWwindow*								getWindow();
		vk::Format								getSwapChainFormat();
		vk::raii::PhysicalDevice*				getPhysicalDevice();
		vk::raii::Instance*						getInstance();
		vk::raii::Queue*						getQueue();
		uint32_t								getQueueFamily();
		vk::raii::Pipeline*						getPipeline();
		uint32_t*								getpFrameIndex();
		std::vector<vk::raii::Image>*			getpImages();
		vk::raii::Image*						getpColorImage();
		vk::raii::DeviceMemory*					getpColorImageMemory();
		vk::raii::ImageView*					getpColorImageView();
		vk::raii::ImageView*					getDepthImageView();
		std::vector<vk::raii::ImageView>*		getSwapChainImageViews();
		uint32_t								getQueueIndex();
		void setGuiCommandBuffers(std::vector<vk::raii::CommandBuffer>* pCmd);
		void setGuiRef(Gui* gui);
	private:
		Gui* gui = nullptr;
		std::vector<const char*> getRequiredInstanceExtensions();
		std::vector<vk::raii::CommandBuffer>* pUiCommandBuffer;
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
		vk::raii::Device device				= nullptr;
		//logical device creation
		vk::raii::Queue queue				= nullptr;
		std::unique_ptr<vk::raii::Queue> computeQueue		= nullptr;
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
		std::vector<vk::Image> swapChainImages{};
		void createImageViews();
		std::vector<vk::raii::ImageView> swapChainImageViews{};

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
		std::vector<vk::raii::CommandBuffer> commandBuffers{};
		

		void createVertexBuffer();
		//void updateVertexBuffer();
		void createIndexBuffer();
		void copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size);
		std::vector<std::vector<Vertex>> vertices{};
		std::vector <std::vector<uint32_t>> indices{};
		vk::raii::Buffer vertexBuffer				= nullptr;
		vk::raii::DeviceMemory vertexBufferMemory	= nullptr;
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);
		vk::raii::Buffer indexBuffer				= nullptr;
		vk::raii::DeviceMemory indexBufferMemory	= nullptr;
		void createUniformBuffers();
		void updateUniformBuffer(uint32_t frameindex);
		std::vector<vk::raii::Buffer> uniformBuffers{};
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory{};
		std::vector<void*> uniformBuffersMapped{};

		void createDescriptorPool();

		vk::raii::DescriptorPool descriptorPool = nullptr;
		void createDescriptorSets();
		//vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> descriptorSets{};


		void createCommandBuffer();
		vk::raii::CommandPool    commandPool = nullptr;
		void recordCommandBuffer(uint32_t imageIndex);
		

		//sync objects
		std::vector<vk::raii::Semaphore> presentCompleteSemaphores{};
		std::vector<vk::raii::Semaphore> renderFinishedSemaphores{};
		std::vector<vk::raii::Fence> inFlightFences{};
		void createSyncObjects();
		//multiple frame handlers
		uint32_t frameIndex = 0;

		//shader loading class
		ShaderLoader shaderLoader{};

		//swap chain recreation
		void recreateSwapChain();
		void cleanupSwapChain();


		//texture stuff
		void createTextureImage(uint32_t index,std::string path);
		uint32_t mipLevels;
		//vk::raii::Image        textureImage = nullptr;
		//std::unique_ptr<vk::raii::Image> textureImage;
		//vk::raii::DeviceMemory textureImageMemory = nullptr;
		std::unique_ptr<vk::raii::CommandBuffer> beginSingleTimeCommands();
		void endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer);
		
		void generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);



		void createTextureImageView(vk::raii::Image* image);
		vk::raii::ImageView createImageView(const vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels);
		void createTextureSampler();
		//vk::raii::ImageView textureImageView	= nullptr;
		vk::raii::Sampler textureSampler		= nullptr;
		
		vk::raii::Image depthImage = nullptr;
		vk::raii::DeviceMemory depthImageMemory = nullptr;
		vk::raii::ImageView depthImageView = nullptr;
		void createDepthResources();
		vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);
		vk::Format findDepthFormat();
		bool hasStencilComponent(vk::Format format);
		//vk::Format depthFormat = findDepthFormat();


		void loadModel(uint32_t index,std::string path);


		vk::SampleCountFlagBits msaaSamples = vk::SampleCountFlagBits::e1;
		vk::SampleCountFlagBits getMaxUsableSampleCount();
		vk::raii::Image colorImage = nullptr;
		vk::raii::DeviceMemory colorImageMemory = nullptr;
		vk::raii::ImageView colorImageView = nullptr;
		void createColorResources();

		void loadModelGLTF();



		//profilin
		float prevtime = 0.0f;


		std::vector<meshObject> meshes;
		void setupMeshes();


		std::vector<std::string> models{"models/viking_room.obj" , "models/Arrow.obj"};
		std::vector<std::string> textures{"textures/viking_room.png","textures/Arrow.png" };
		void prepareModels();

		std::vector<vk::raii::Image> imagesArr{};
		std::vector<vk::raii::DeviceMemory> imageMem{};
		std::vector<vk::raii::ImageView> imageViewArr{};


		void createParticleBuffer();
		int particle_count = 120;
		void createComputePipeline();
		std::vector<vk::raii::Buffer> shaderStorageBuffers{};
		std::vector<vk::raii::DeviceMemory> shaderStorageBuffersMemory{};
		void createComputeShaderDescriptorSetLayout();
		vk::raii::DescriptorSetLayout computeDescriptorSetLayout = nullptr;
		void createComputeDescriptorSets();
		void createComputeDescriptorPool();
		vk::raii::DescriptorPool computeDescriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> computeDescriptorSets{};

		uint64_t frameNumber = 0;
	};

	


}
