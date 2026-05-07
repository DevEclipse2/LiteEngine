#pragma once
#include "Lt_MultiWindow.h"
#include <vector>
#include <string>
namespace lte {

	struct Lt_WindowInfo {
		std::string displayName = "";
		std::string internalName = "";
		uint16_t width = 0;
		uint16_t height = 0;
		uint16_t ID = 0;
		uint16_t InfoIndex = 0;
	};

	class Lt_WindowTracker
	{
		public:
		static char WindowCount;
		static std::vector<std::unique_ptr<Lt_MultiWindow>> windows;
		static std::vector<std::string> windowInfo;
		void createSubWindow();
		void createMainWindow(Lt_WindowInfo& info);
		static int mainId;
		void MergeSubSub();
		void MergeSubMain();
		private:
			void createWindow(Lt_WindowInfo& info);
	};
}