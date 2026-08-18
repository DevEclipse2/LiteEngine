#include "windowTracker.h"
namespace ltCore {
	void windowTracker::Init()
	{
		glfwSetErrorCallback(glfwErrorCallback);

		if (!glfwInit()) {
			std::cerr << "Failed to initialize GLFW!" << std::endl;
			return;
		}
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	}
	void windowTracker::DefaultWindow()
	{
		mainWindowIndex = CreateWindow(800,600,"LiteNgine");
	}
	uint32_t windowTracker::CreateWindow(int width, int height, std::string name)
	{
		Lt_window window{};
		window.width = width;
		window.height = height;
		window.id = counter;
		window.window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
		SubWindows[counter] = std::make_unique<Lt_window>(window);
		counter++;
		return counter - 1;
	}
	void windowTracker::frameBufferResizeCallback(GLFWwindow* window, int width, int height)
	{
	}
}