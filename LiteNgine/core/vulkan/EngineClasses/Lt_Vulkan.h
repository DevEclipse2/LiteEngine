#pragma once
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <limits>
#include <vector>
#include "../Reworked/DebugMessenger.h"
#include "../Reworked/DeviceHandler.h"
#include "Lt_MultiWindow.h"
#include "../Reworked/LtSync.h"
#include "../Reworked/FileLoader.h"
#include "Lt_WindowTracker.h"
#define FOUND_DEVICES_HIGHER_THAN_EXPECTED	1;
#define FOUND_DEVICES_LOWER_THAN_EXPECTED	2;
//#define FOUND_DEVICES_NO_VULKAN_SUPPORT		= 3;
#define FOUND_DEVICES_NO_PRESENT_SUPPORT	4;
#define FOUND_DEVICES_NO_DEVICES			5;
#define FOUND_DEVICES_NONE_SUITABLE			6;

#define CREATE_DEPTH_IMG					1;
#define CREATE_COLOR_IMG					2;



//the all in one class that does only one thing and one thing badly
//then it fucks off
namespace lte {
	class DeviceHandler;

	struct Lt_DevicePair {
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device logicalDevice = nullptr;
		vk::SampleCountFlagBits sampling;
		vk::raii::Queue queue = nullptr;
		uint16_t queueIndex = 0;
	};
	struct Lt_WindowVK
	{
		//constructor for stuff i guess
		Lt_WindowVK();
		//heres what you need multiple of for windows:
		//surfaces
		vk::raii::SurfaceKHR surface = nullptr;
		//pipelines
		LtPipeline pipeline{};
		LtSwapChain swapchain{};
		//commandbuffer
		std::vector<vk::CommandBuffer> Commands = {};
		std::vector<vk::raii::CommandBuffer> cmdBuffer = {};
		//this is something to help transition the images properly


		//sync stuff
		LtSyncSet syncSet{};
		//registry for that index
		uint32_t ltMultiWindowIndex = 0;
		uint32_t deviceID = 0;
		vk::Extent2D swapChainExtent;
		uint32_t minImageCount = 0;
		int width = 800;
		int height = 600;
		void registerWindow(uint32_t& windowIndex, uint32_t glfwWindowIndex);
		void createSwapChain(Lt_MultiWindow& window);
		void recreateSwapChain	();
		void addCommand(vk::raii::CommandBuffer& commandBuffer);
		void addCommand(vk::CommandBuffer& commandBuffer);
		void newFrame(uint8_t index);
		//these are called globally
		void resetBuffers();
		void submitBuffers(uint8_t index);
		void startRender(uint8_t index);
		void prepCommand(uint8_t frame);
		uint32_t availableIndex = 0;
	};

	class DebugMessenger; 


	class Lt_Vulkan
	{
	public:
		void Init(std::string name);

		static vk::raii::CommandPool commandPool;

		static std::vector<Lt_WindowVK> windows;

		static std::vector<Lt_DevicePair> devices;

		static std::vector<void (*)()> resetFunction;
		static std::vector<void (*)()> submitFunction;

		static void createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window);

		static vk::raii::Instance instance;

		static vk::raii::Sampler sampler;

		static uint8_t FramesInFlight;


	private:
		static vk::raii::SurfaceKHR TempSurface;
		static vk::raii::Context context;
		static std::vector<const char*> getRequiredInstanceExtensions(bool enableValidationLayers);
		static void createInstance(std::string name, bool useValidationLayers);
		


		static std::vector<std::vector<vk::CommandBuffer>> commandBuffer; // one set for each

		const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };


		DebugMessenger messenger{};
		static const std::vector<char const*> validationLayers;

		//one please

	};
}