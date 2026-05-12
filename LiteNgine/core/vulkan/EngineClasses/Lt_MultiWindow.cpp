#include "Lt_MultiWindow.h"
namespace lte {
	GLFWwindow* Lt_MultiWindow::resizedWindow = nullptr;
	//void (Lt_MultiWindow::*resizeCallback)() = nullptr;
	void (*Lt_MultiWindow::resizeCallback)(void) = nullptr;
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

	void Lt_MultiWindow::GetWindowSize(int& retwidth, int& retheight)
	{
		retwidth = width;
		retheight = height;
	}

	void Lt_MultiWindow::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
		//vkdevice->keyCallback(window, key, scancode, action, mods);
	}
	void Lt_MultiWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
		resizedWindow = window;
		if (resizeCallback != nullptr) {
			resizeCallback();
		}
		else {
			std::cout << "no assigned Callback!" << std::endl;
		}
		//vkdevice->framebufferResized = true;
	}
	GLFWwindow* Lt_MultiWindow::getGLFWWindow() {
		return window;
	}

	void Lt_MultiWindow::charCallback(GLFWwindow* window, unsigned int codepoint) {
		//ImGui_ImplGlfw_CharCallback(window, codepoint);
		//vkdevice->charCallback(window, codepoint);
	}
}