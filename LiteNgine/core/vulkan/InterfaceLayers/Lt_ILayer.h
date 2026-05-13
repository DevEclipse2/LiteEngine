#pragma once
#include "../Reworked/FileLoader.h"
#include "../EngineClasses/Lt_WindowTracker.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Gui.h"

//this is where the main function comes to meet with the usable code
namespace lte {
	
	class LtBackend;
	class Lt_WindowTracker;
	class Lt_Vulkan;
	class Lt_Gui;
	class Lt_ILayer
	{
	public:
		void Begin();
		void End();
		void Cleanup();
		void Loop();
		void Resize();

	private:
		//Lt_Window ltWindow{ 800, 600 ,"LiteEngine : Agstrum"};
		FileLoader fileLoader{};
		Lt_WindowTracker windowMgr{};
		Lt_Vulkan vulkanHandler{};
		Lt_Gui guiHandler{};

		uint32_t mainWindowIndex = 0;
		uint8_t frames = 0;

		bool mainResized = false;
	};
}