#include "App.h"
#include "Lt_Window.h"
namespace lte {
	void main::run() {

		ltWindow.setVkDevice(&vkDevice);

		while (!ltWindow.shouldClose()) {
			glfwPollEvents();
			vkDevice.drawFrame();
		}
		vkDevice.Exit();
	}
}