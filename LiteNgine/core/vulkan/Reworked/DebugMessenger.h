#pragma once
#include "vulkan/vulkan_raii.hpp"
#include <iostream>
#include <algorithm>
#include <cstdlib>
namespace lte {
	class DebugMessenger
	{
	public:
		DebugMessenger();
		~DebugMessenger();
		void setupMessenger(vk::raii::Instance* instance);
		vk::DebugUtilsMessengerEXT debugMessenger;
	};
}

