#include "App.h"
namespace lte {
	void main::run() {

		InterfaceLayer.Begin();
		/*
		ltWindow.setVkDevice(&vkDevice);
		vkDevice.setGuiRef(&uiProc);
		vkDevice.setGuiCommandBuffers(uiProc.getpCommandBuffers());
		uiProc.setLayoutManager(&uiLayout);*/
		pWindow = &InterfaceLayer.backend.window;
		while (!pWindow->shouldClose()) {
			glfwPollEvents();
			InterfaceLayer.Loop();
		}
		InterfaceLayer.End();
		//vkDevice.Exit();
	}
}