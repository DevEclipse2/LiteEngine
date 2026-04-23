#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan_raii.hpp>
/*#include "VulkanDevice.h"*/
namespace lte {
	class VulkanDevice;
	class Lt_Window
	{
	public:
		Lt_Window(int w, int h, std::string name);
		~Lt_Window();
		Lt_Window(const Lt_Window&) = delete;
		Lt_Window& operator=(const Lt_Window&) = delete;
		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		GLFWwindow* getGLFWWindow();
		static bool Resized;
		//void setVkDevice(VulkanDevice* device);
	private:

		//static VulkanDevice* vkdevice;
		GLFWwindow* window;
		void initWindow();
		const int width;
		const int height;
		std::string windowName;
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void charCallback(GLFWwindow* window, unsigned int codepoint);


	};
}

