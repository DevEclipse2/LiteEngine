#pragma once
#include "Lt_MultiWindow.h"
#include <vector>
#include <string>
namespace lte {

	struct Lt_WindowInfo {
		Lt_MultiWindow window{};
		std::string displayName = "";
		std::string internalName = "";
		uint16_t width = 0;
		uint16_t height = 0;
		std::vector<void(*)()> resizePointers = {};
	};

	class Lt_WindowTracker
	{
		public:
			void Startup();
			static char WindowCount;
			static std::vector<std::unique_ptr<Lt_WindowInfo>> windowInfo;
			void createSubWindow();
			void createMainWindow(Lt_WindowInfo& info);
			static int mainId;
			void MergeSubSub();
			void MergeSubMain();
			static void WindowResized();
		private:
			void createWindow(Lt_WindowInfo& info);
	};
}