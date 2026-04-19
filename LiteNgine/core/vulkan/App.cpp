#include "App.h"
#include "Lt_Window.h"
namespace lte {
	void main::run() {

		ltWindow.setVkDevice(&vkDevice);
		vkDevice.setGuiRef(&uiProc);
		vkDevice.setGuiCommandBuffers(uiProc.getpCommandBuffers());
		uiProc.setLayoutManager(&uiLayout);
		while (!ltWindow.shouldClose()) {
			glfwPollEvents();
			vkDevice.drawFrame();
		}
		vkDevice.Exit();
	}
}