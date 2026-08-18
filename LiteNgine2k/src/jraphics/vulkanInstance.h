#pragma once
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "DebugMessenger.h"
#include "Devicehandler.h"
namespace ltCore {


	//these are for the windows that tie to glfw stuffs
	struct VulkanWindow
	{

	};


	class vulkanInstance
	{
		vk::raii::Context m_context;
		vk::raii::Instance m_instance = nullptr;
		void createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window);
		void createInstance(std::string name, bool useValidationLayers);
		std::vector<const char*> getRequiredInstanceExtensions(bool enableValidationLayers);
		void Init(std::string name);
		DebugMessenger messenger{};
		Devicehandler handler{};
		vk::raii::SurfaceKHR tempSurface = nullptr;

		//for creation


		const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
		static const std::vector<char const*> validationLayers;
	};
}
