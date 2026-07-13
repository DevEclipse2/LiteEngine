#include "App.h"
namespace lte {
	void main::run() {

		try {
			
		
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
		Con::LogEvent("engine shutdown-------------------------------------------", TAG_ENGINE);
		InterfaceLayer.End();
		Con::LogEvent("engine cleanup-------------------------------------------", TAG_ENGINE);
		InterfaceLayer.Cleanup();
		Con::OutputFile();
		}
		catch (const vk::SystemError& err) {
			std::cerr << "Vulkan Error Caught: " << err.what() << "\n";
			Con::Display();
			Con::OutputFile();
		}
		catch (const std::exception& err) {
			std::cerr << "Standard C++ Error: " << err.what() << "\n";
			Con::Display();
			Con::OutputFile();
		}
	}
}