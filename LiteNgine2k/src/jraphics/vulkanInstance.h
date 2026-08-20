#pragma once
#include "vulkan/vulkan_raii.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "DebugMessenger.h"
#include "Devicehandler.h"
#define FIF 3
namespace ltCore {


	//these are for the windows that tie to glfw stuffs
	struct VulkanWindow
	{

	};


	class vulkanInstance
	{
	public:
		vk::raii::Context m_context;
		vk::raii::Instance m_instance = nullptr;
		
		std::vector<const char*> getRequiredInstanceExtensions(bool enableValidationLayers);
		void Init(std::string name);
		DebugMessenger messenger{};
		Devicehandler handler{};
		vk::raii::SurfaceKHR tempSurface = nullptr;
		void createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window);
		//for creation
		void drawFrame();
		uint32_t frameIndex;
		const std::vector<const char*> requiredDeviceExtensions = { vk::KHRSwapchainExtensionName };
		const std::vector<char const*> validationLayers;

	private:
		void createInstance(std::string name, bool useValidationLayers);
	};
}
