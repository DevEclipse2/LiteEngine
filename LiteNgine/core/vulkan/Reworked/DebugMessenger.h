#pragma once
#include "vulkan/vulkan_raii.hpp"
#include <iostream>
namespace lte {
	class DebugMessenger
	{
	public:
		DebugMessenger();
		~DebugMessenger();
		void setupMessenger(vk::raii::Instance* instance);
		vk::DebugUtilsMessengerEXT debugMessenger;
		VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
			vk::DebugUtilsMessageTypeFlagsEXT              type,
			const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
	};
}

