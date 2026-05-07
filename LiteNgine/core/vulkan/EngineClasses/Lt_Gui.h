#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include "../../../dep/backends/imgui.h"
#include "../../../dep/backends/imgui_impl_glfw.h"
#include "../../../dep/backends/imgui_impl_vulkan.h"
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
//this should be a static thing and should only have 1! instance ever
//or at most 2 because i cannot tell you what to do.
// a slightly better (or worse) implementation of GUI
namespace lte 
{
	class Lt_Gui
	{

		void Instantiate();
		bool drawFrame();
		void updateBuffers();
		void updateFrameBuffer();
		bool firstFrame = true;
		void handleKey(int key, int scancode, int action, int mods);
		void charPressed(uint32_t key);
		bool getWantKeyCapture();
		public:

	};

}