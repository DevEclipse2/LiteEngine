#include "Lt_WindowTracker.h"
namespace lte {
	 std::vector<std::unique_ptr<Lt_MultiWindow>> Lt_WindowTracker::windows = {};
	 std::vector<std::string> Lt_WindowTracker::windowInfo = {};
	 int Lt_WindowTracker::mainId = -2;
	 void Lt_WindowTracker::createMainWindow(Lt_WindowInfo& info)
	 {
		 createWindow(info);
		 mainId = info.ID;
	 }
	 void Lt_WindowTracker::createWindow(Lt_WindowInfo& info)
	{
		Lt_MultiWindow window{};
		window.CreateWindow(info.width, info.height,info.displayName);
		info.ID = windows.size();
		windows.emplace_back(std::make_unique<Lt_MultiWindow>(std::move(window)));
		info.InfoIndex = windowInfo.size();
		windowInfo.emplace_back(info.internalName);
		// TODO: make vulkan surface and stuff for this
	}
}