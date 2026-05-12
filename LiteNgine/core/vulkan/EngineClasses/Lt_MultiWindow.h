#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>

//can you believe it? the Lt_window class is already deprecated after what, like 2 months?
//my code is bad
namespace lte
{
	class Lt_MultiWindow
	{
	public:
		/*Lt_MultiWindow();
		~Lt_MultiWindow();*/
		/*
		Lt_MultiWindow(const Lt_MultiWindow&) = delete;
		Lt_MultiWindow& operator=(const Lt_MultiWindow&) = delete;*/
		static GLFWwindow* resizedWindow;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		GLFWwindow* getGLFWWindow();
		bool thisResized = false;
		
		void CreateWindow(int width, int height, std::string name);
		void DestroyWindow();
		void GetWindowSize(int& width, int& height);
		static void (*resizeCallback)();
		int width = 800;
		int height = 600;
	private:

		GLFWwindow* window = nullptr;
		
		std::string windowName = "You forgot to name this window properly";


		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void charCallback(GLFWwindow* window, unsigned int codepoint);
	};
}
