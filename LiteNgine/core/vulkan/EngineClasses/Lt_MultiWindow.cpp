#include "Lt_MultiWindow.h"
namespace lte {
	GLFWwindow* Lt_MultiWindow::resizedWindow = nullptr;
	bool Lt_MultiWindow::resized = false;

	void Lt_MultiWindow::CreateWindow(int w, int h, std::string name) {
		width = w;
		height = h;
		windowName = name;
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
	void Lt_MultiWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		//vkdevice->keyCallback(window, key, scancode, action, mods);
	}

	void Lt_MultiWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		resizedWindow = window;
		resized = true;
		//vkdevice->framebufferResized = true;
	}
	void Lt_MultiWindow::charCallback(GLFWwindow* window, unsigned int codepoint) {
		//ImGui_ImplGlfw_CharCallback(window, codepoint);
		//vkdevice->charCallback(window, codepoint);
	}
}