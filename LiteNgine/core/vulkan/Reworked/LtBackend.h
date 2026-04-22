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

namespace lte {
	class DebugMessenger;
	class Lt_Window;
	class DeviceHandler;
	struct BackendInitInfo {
		int width, height;
		std::string WindowName = "";
		bool useValidationLayers;
		std::string name = "";
	};
	class LtBackend
	{
		
		public:
			LtBackend(BackendInitInfo info);
			~LtBackend();
			void InitializeVulkan(BackendInitInfo info);

			//might be unsafe
			void AssignDrawPtr(LtMeshInfo* ptr);
			
		private:
			int width;
			int height;
			std::string name;
			Lt_Window window{width, height,name};
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
			vk::raii::PhysicalDevice PhysicalDevice = nullptr;
			vk::raii::SurfaceKHR surface = nullptr;
			vk::SampleCountFlagBits msaaSamples;
			LogicalDevice primary{};
			LtSwapChain swapchain{nullptr,nullptr,nullptr,nullptr,nullptr};
			//LtPipeline mainPipeline{};
			LtImage* colorImage = nullptr;
			LtImage* depthImage = nullptr;
			LtPipeline pipeline{};
			uint32_t minImageCount = 0;
			vk::raii::CommandPool commandPool = nullptr;

			//this is for drawing
			//dont @me on this
			LtMeshInfo* drawPtr = nullptr;
			char drawList[6] = {255,255,255 , 255, 255 ,255};
			uint64_t objects = 0;
			void updateDrawCount();
			

			//indices and vertices will be in c style array
			Vertex		vertexes[1] = {{}};
			uint32_t	indices[];
			
	};

}
