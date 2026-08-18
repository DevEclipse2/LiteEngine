#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include "../forScrap/Lt_Console.h"
namespace ltCore {
	
	
	struct Lt_window
	{
		GLFWwindow* window;
		int width = 0;
		int height = 0;
		int id;
		bool shouldClose()
		{
			return glfwWindowShouldClose(window);
		}
	};
	class windowTracker
	{
	public:
		static void glfwErrorCallback(int error, const char* description) {
		lte::Con::LogError("GLFW Error (" + std::to_string(error) + "): " + description, HIGH_SEVERITY, TAG_ENGINE);
		}
		static void Init();
		static void DefaultWindow();
		static uint32_t CreateWindow(int width, int height, std::string name);
		static void frameBufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void dropCallback(GLFWwindow* window, int count, const char** paths);
		//cannot be copied so uh use unique ptrs
		inline static uint32_t counter; // increment when new subwindow
		inline static std::unordered_map<uint32_t,std::unique_ptr<Lt_window>> SubWindows;
		inline static uint32_t mainWindowIndex;
	};
}

