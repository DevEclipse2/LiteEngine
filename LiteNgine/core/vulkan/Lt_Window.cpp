#include "Lt_Window.h"
#include <string>
#include <stdexcept>
namespace lte {

	int Lt_Window::width = 0;
	int Lt_Window::height = 0;
	//VulkanDevice* Lt_Window::vkdevice = nullptr;
	bool Lt_Window::Resized = false;

	void Lt_Window::CreateWindow(int w, int h, std::string name){
		width = w;
		height = h;
		windowName = name;
		initWindow();
	}
	void Lt_Window::DestroyWindow()
	{
		if (window) {
			glfwDestroyWindow(window);
		}
		glfwTerminate();
	}
	Lt_Window::Lt_Window()
	{

	}
	Lt_Window::~Lt_Window(){
		
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
		//vkdevice->keyCallback(window, key, scancode, action, mods);
	}
	
	void Lt_Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		Resized = true;
		//vkdevice->framebufferResized = true;
	}
	void Lt_Window::charCallback(GLFWwindow* window, unsigned int codepoint) {
		//ImGui_ImplGlfw_CharCallback(window, codepoint);
		//vkdevice->charCallback(window, codepoint);
	}

}