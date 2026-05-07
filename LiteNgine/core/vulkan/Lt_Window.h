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
		Lt_Window();
		~Lt_Window();
		Lt_Window(const Lt_Window&) = delete;
		Lt_Window& operator=(const Lt_Window&) = delete;
		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		GLFWwindow* getGLFWWindow();
		static bool Resized;
		void CreateWindow(int width, int height, std::string name);
		void DestroyWindow();
		//void setVkDevice(VulkanDevice* device);
	private:

		//static VulkanDevice* vkdevice;
		GLFWwindow* window = nullptr;
		void initWindow();
		static int width;
		static int height;
		std::string windowName = "name";
		static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
		static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void charCallback(GLFWwindow* window, unsigned int codepoint);


	};
}

