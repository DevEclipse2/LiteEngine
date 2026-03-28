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
#include <map>
namespace lte {
	class VulkanDevice
	{
	public:
		#ifdef NDEBUG
			static constexpr bool enableValidationLayers = false;
		#else
			static constexpr bool enableValidationLayers = true;
		#endif
		VulkanDevice();
		~VulkanDevice();
		const std::vector<char const*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
			};
	private:
		std::vector<const char*> getRequiredInstanceExtensions();
		vk::raii::Context  context;
		vk::raii::Instance instance = nullptr;
		int createInstance();
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
			vk::DebugUtilsMessageTypeFlagsEXT              type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		void setupDebugMessenger();
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
		void pickPhyscialDevice();
		vk::raii::PhysicalDevice physicalDevice = nullptr;
		bool isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice);
	};
}
