#pragma once
#include "vulkan/vulkan_raii.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
	vk::DebugUtilsMessageTypeFlagsEXT              type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData);
namespace ltCore {

	class DebugMessenger
	{
	public:
		void setupMessenger(vk::raii::Instance& instance);
		vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	};
}

