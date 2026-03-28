#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <vulkan/vulkan_raii.hpp>
namespace lte {

	class Lt_Window
	{
	public:
		Lt_Window(int w, int h, std::string name);
		~Lt_Window();
		Lt_Window(const Lt_Window&) = delete;
		Lt_Window& operator=(const Lt_Window&) = delete;
		bool shouldClose() { return glfwWindowShouldClose(window); }
		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
	private:

		GLFWwindow* window;
		void initWindow();
		const int width;
		const int height;
		std::string windowName;


	};
}

