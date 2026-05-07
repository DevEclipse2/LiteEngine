#pragma once
#include "../Reworked/LtBackend.h"
#include "../Reworked/FileLoader.h"
#include "../Lt_Window.h"
#include "../EngineClasses/Lt_WindowTracker.h"
#include "../EngineClasses/Lt_Vulkan.h"

//this is where the main function comes to meet with the usable code
namespace lte {
	
	class LtBackend;
	class Lt_WindowTracker;
	class Lt_Vulkan;
	class Lt_ILayer
	{
		public:
			void Begin();
			void End();
			void Cleanup();
			void Loop();
			LtBackend backend{};
		private:
			//Lt_Window ltWindow{ 800, 600 ,"LiteEngine : Agstrum"};
			FileLoader fileLoader{};
			Lt_WindowTracker windowMgr{};
			Lt_Vulkan vulkanHandler{};
	};
}