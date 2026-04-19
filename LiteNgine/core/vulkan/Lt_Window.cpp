#include "Lt_Window.h"
#include <string>
#include <stdexcept>
namespace lte {

	VulkanDevice* Lt_Window::vkdevice = nullptr;

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
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
		glfwSetKeyCallback(window, KeyCallback);
		glfwSetCharCallback(window, charCallback);
			/*glfwSetScrollCallback
		glfwGetCursorPos*/
		
	}
	void Lt_Window::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		vkdevice->keyCallback(window, key, scancode, action, mods);
	}
	
	void Lt_Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		vkdevice->framebufferResized = true;
	}
	void Lt_Window::setVkDevice(VulkanDevice* device){
		vkdevice = device;
	}
	void Lt_Window::charCallback(GLFWwindow* window, unsigned int codepoint) {
		//ImGui_ImplGlfw_CharCallback(window, codepoint);
		vkdevice->charCallback(window, codepoint);
	}

}