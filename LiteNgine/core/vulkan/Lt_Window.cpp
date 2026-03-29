#include "Lt_Window.h"
#include <string>
#include <stdexcept>
namespace lte {

	Lt_Window::Lt_Window(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}
	Lt_Window::~Lt_Window(){
		if (window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	};
	GLFWwindow* Lt_Window::getGLFWWindow() {
		return window;
	}
	void Lt_Window::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {

		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create windows surface. Chat are we cooked?");
		}
	}
	void Lt_Window::initWindow(){
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	}
}