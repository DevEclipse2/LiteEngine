#include "App.h"
namespace lte {
	void main::run() {

		InterfaceLayer.Begin();
		/*
		ltWindow.setVkDevice(&vkDevice);
		vkDevice.setGuiRef(&uiProc);
		vkDevice.setGuiCommandBuffers(uiProc.getpCommandBuffers());
		uiProc.setLayoutManager(&uiLayout);*/
		mainWindow = &Lt_WindowTracker::windowInfo[InterfaceLayer.mainWindowIndex]->window;
		while (!mainWindow->shouldClose()) {
			glfwPollEvents();
			InterfaceLayer.Loop();
		}
		InterfaceLayer.End();
		//vkDevice.Exit();
	}
}