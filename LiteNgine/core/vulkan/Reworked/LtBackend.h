//this replaces vulkandevice as the new way to do, well anything

#pragma once
#include "LtMesh.h"
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "../Lt_Window.h"
#include "DebugMessenger.h"
#include "DeviceHandler.h"
#include "PipelineDelegate.h"
#include "SwapchainHandler.h"
#include "ImageDelegate.h"
#include "CommandBuffers.h"
#include "FileLoader.h"
#include "LtSync.h"
#include "TemporaryDraw.h"
namespace lte {
	class DebugMessenger;
	class Lt_Window;
	class DeviceHandler;
	struct BackendInitInfo {
		int width, height;
		std::string WindowName = "";
		bool useValidationLayers;
		std::string name = "";
		Lt_Window* window = nullptr;
	};
	class LtBackend
	{
		
		public:
			LtBackend();
			~LtBackend();
			void InitializeVulkan(BackendInitInfo info);

			//might be unsafe
			void AssignDrawPtr(LtMeshInfo* ptr);
			
			//temporary measure 
			LogicalDevice primary{};
			vk::raii::PhysicalDevice PhysicalDevice = nullptr;
			vk::raii::CommandPool commandPool = nullptr;
			void second();

			void Update();

			void Exit();

			void Draw();

			int width = 800;
			int height = 600;
			std::string name = "window";

			Lt_Window window{ width, height,name };
		private:
			void createSurface();
			void createInstance(BackendInitInfo info);
			void RegisterGameObjects();
			const std::vector<char const*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
			};
			std::vector<const char*> getRequiredInstanceExtensions(bool enableValidationLayers);
			const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
			DebugMessenger messenger{};
			DeviceHandler deviceHandler{};
			vk::raii::Context context;
			vk::raii::Instance instance = nullptr;
			vk::raii::SurfaceKHR surface = nullptr;
			vk::SampleCountFlagBits msaaSamples;
			LtSwapChain swapchain{nullptr,nullptr,nullptr,nullptr,nullptr};
			//LtPipeline mainPipeline{};
			uint32_t colorImageIndex = 0;
			uint32_t depthImageIndex = 0;
			LtPipeline pipeline{};
			uint32_t minImageCount = 0;

			//this is for drawing
			//dont @me on this
			LtMeshInfo* drawPtr = nullptr;
			char drawList[6] = { 'c','c','c', 'c', 'c' ,'c'};
			uint64_t objects = 0;
			void updateDrawCount();
			uint8_t framesInFlight = 3;
			uint32_t maxObjects = 2048;
			vk::raii::Sampler sampler = nullptr;


			vk::raii::Buffer vertexBuffer = nullptr;
			vk::raii::DeviceMemory vertexBufferMem = nullptr;

			vk::raii::Buffer indexBuffer = nullptr;
			vk::raii::DeviceMemory indexBufferMem = nullptr;
			std::vector<RenderSet> renderSets = {};
			std::vector<LtMeshInfo> MeshInfo = {};
			
			LtSyncSet synchronizationSet{};

			vk::raii::DescriptorPool pool = nullptr;
			std::vector<vk::raii::CommandBuffer> commandBuffers = {};

			bool framebufferResized = false;
			uint32_t availableIndex = 0;
			uint32_t frameNumber = 0;
			uint8_t frameIndex = 0;
			void recreateSwapChain();

	};

}
