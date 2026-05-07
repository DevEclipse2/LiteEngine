#pragma once
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <limits>
#include <vector>
#include "../Reworked/DebugMessenger.h"
#include "../Reworked/DeviceHandler.h"


#define FOUND_DEVICES_HIGHER_THAN_EXPECTED	1;
#define FOUND_DEVICES_LOWER_THAN_EXPECTED	2;
//#define FOUND_DEVICES_NO_VULKAN_SUPPORT		= 3;
#define FOUND_DEVICES_NO_PRESENT_SUPPORT	4;
#define FOUND_DEVICES_NO_DEVICES			5;
#define FOUND_DEVICES_NONE_SUITABLE			6;


//the all in one class that does only one thing and one thing badly
//then it fucks off
namespace lte {
	class DeviceHandler;

	struct Lt_DevicePair {
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		vk::raii::Device logicalDevice = nullptr;
		vk::SampleCountFlagBits sampling;
		vk::raii::Queue queue = nullptr;
		uint16_t queueIndex;
	};
	struct Lt_WindowVK
	{
		//heres what you need multiple of for windows:
		//surfaces
		vk::raii::SurfaceKHR surface = nullptr;
		//pipelines
		//commandbuffer
		//sync stuff

	};
	class DebugMessenger; 
	class Lt_Vulkan
	{
	public:
		void Init(std::string name);

		void createSurfaces();
		static std::vector<std::unique_ptr<Lt_WindowVK>> windows;
	private:
		static std::vector<std::unique_ptr<Lt_DevicePair>> devices;
		static vk::raii::SurfaceKHR TempSurface;
		static vk::raii::Context context;
		static vk::raii::Instance instance;
		static std::vector<const char*> getRequiredInstanceExtensions(bool enableValidationLayers);
		static void createInstance(std::string name, bool useValidationLayers);
		static void createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window);
		const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };


		DebugMessenger messenger{};
		static const std::vector<char const*> validationLayers;
		DeviceHandler handler{};

	};
}