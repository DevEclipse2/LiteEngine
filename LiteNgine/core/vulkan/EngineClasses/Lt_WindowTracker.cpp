#include "Lt_WindowTracker.h"
namespace lte {
	 std::vector<std::unique_ptr<Lt_WindowInfo>> Lt_WindowTracker::windowInfo = {};
	 int Lt_WindowTracker::mainId = -2;
	 void Lt_WindowTracker::Startup()
	 {
		 Lt_MultiWindow::resizeCallback = WindowResized;
	 }
	 void Lt_WindowTracker::createMainWindow(Lt_WindowInfo& info)
	 {
		 createWindow(info);
		 mainId = 0;
	 }
	 void Lt_WindowTracker::WindowResized()
	 {
		//iterates through
		 char windowIndex = 0;
		for (const auto& windowstruct : windowInfo) {
			auto& window = windowstruct->window;
			if (window.getGLFWWindow() == Lt_MultiWindow::resizedWindow) 
			{ 
				window.thisResized = true;
				glfwGetWindowSize(Lt_MultiWindow::resizedWindow,&window.width,&window.height);
				Lt_MultiWindow::resizedWindow = nullptr;
				for(uint16_t funcIndex = 0; funcIndex < windowInfo[windowIndex]->resizePointers.size(); funcIndex++)
				{
					windowInfo[windowIndex]->resizePointers[funcIndex]();
				}
				break;
			}
			windowIndex++;
		}
	 }
	 void Lt_WindowTracker::createWindow(Lt_WindowInfo& info)
	{
		Lt_MultiWindow window{};
		window.CreateWindow(info.width, info.height,info.displayName);
		info.window = std::move(window);
		windowInfo.emplace_back(std::make_unique<Lt_WindowInfo>(info));
		// TODO: make vulkan surface and stuff for this
	}
}