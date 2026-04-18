#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include "../vulkan/VulkanDevice.h"
#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include "../../dep/backends/imgui.h"
#include "../../dep/backends/imgui_impl_glfw.h"
#include "../../dep/backends/imgui_impl_vulkan.h"

#include <GLFW/glfw3.h>
namespace lte {
	class VulkanDevice;
	class Gui
	{
	public:
		Gui(VulkanDevice* vulkDev);
		~Gui();
		
		//void Destroy();

		//vk::CommandBuffer PrepareBuffer(uint32_t Image);
		bool drawFrame();
		void updateBuffers();
		std::vector<vk::raii::CommandBuffer>* getpCommandBuffers();
	private:

		// Dynamic state tracking for performance optimization
		bool needsUpdateBuffers = false;                        // Flag indicating buffer resize requirements

		// UI state management and rendering configuration
		// These members control the visual appearance and dynamic behavior of the UI system
		ImGuiStyle vulkanStyle;                                 // Custom visual styling for Vulkan applications

		// Push constants for efficient per-frame parameter updates
		// This structure enables fast updates of transformation and styling data
		struct PushConstBlock {
			glm::vec2 scale;                                    // UI scaling factors for different screen sizes
			glm::vec2 translate;                                // Translation offset for UI positioning
		} pushConstBlock;

		// Modern Vulkan rendering configuration
		vk::PipelineRenderingCreateInfo renderingInfo{};        // Dynamic rendering setup parameters
		vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;   // Target framebuffer format


		void createDescriptorPool();
		void InitGUI();
		void setStyle(uint32_t index);
		void initResources();
		VulkanDevice* pDevice = nullptr;
		int fbWidth;
		int fbHeight;
		vk::raii::Sampler sampler{ nullptr };                    // Texture sampling configuration for font rendering
		vk::raii::Buffer vertexBuffer = nullptr;                                    // Dynamic vertex buffer for UI geometry
		vk::raii::Buffer indexBuffer = nullptr;                                     // Dynamic index buffer for UI triangle connectivity
		uint32_t vertexCount = 0;                              // Current vertex count for draw commands
		uint32_t indexCount = 0;                               // Current index count for draw commands
		vk::raii::Image fontImage = nullptr;                                        // GPU texture containing ImGui font atlas
		vk::raii::DeviceMemory fontMemory = nullptr;                                        // GPU texture containing ImGui font atlas
		vk::raii::ImageView fontImageView = nullptr;                                // Shader-accessible view of font texture

		// Vulkan pipeline infrastructure for UI rendering
		// These objects define the complete GPU processing pipeline for ImGui elements
		vk::raii::PipelineCache pipelineCache{ nullptr };        // Pipeline compilation cache for faster startup
		vk::raii::PipelineLayout pipelineLayout{ nullptr };      // Resource binding layout (textures, uniforms)
		vk::raii::Pipeline* pipeline{ nullptr };                  // Complete graphics pipeline for UI rendering
		vk::raii::DescriptorPool descriptorPool{ nullptr };      // Pool for allocating descriptor sets
		vk::raii::DescriptorSetLayout descriptorSetLayout{ nullptr }; // Layout defining shader resource bindings
		vk::raii::DescriptorSet descriptorSet{ nullptr };        // Actual resource bindings for font texture

		// Vulkan device context and system integration
		// These references connect our UI system to the broader Vulkan application context
		vk::raii::Device* device = nullptr;                    // Primary Vulkan device for resource creation
		vk::raii::PhysicalDevice* physicalDevice = nullptr;    // GPU hardware info for capability queries
		vk::raii::Queue* graphicsQueue = nullptr;              // Command submission queue for UI rendering
		uint32_t graphicsQueueFamily = 0;                      // Queue family index for validation
		std::vector<vk::raii::CommandBuffer> commandBuffers{};
		std::vector<vk::raii::Image>* pImages;
		void recordCommandBuffer(ImDrawData* data , uint8_t index);
		void handleKey(int key, int scancode, int action, int mods);
		void charPressed(uint32_t key);
		bool getWantKeyCapture();
		bool showWindow;
		uint32_t* pFrameIndex;

		void getPtrs();
		vk::raii::Image*		pColorImage;
		vk::raii::DeviceMemory* pColorImageMemory;
		vk::raii::ImageView*	pColorImageView;
		vk::raii::ImageView*	pDepthImageView;
		std::vector<vk::raii::ImageView>* swapChainImageViews;


		vk::raii::CommandPool commandPool = nullptr;
	};
}


