#pragma once
#include "../Reworked/FileLoader.h"
#include "../EngineClasses/Lt_WindowTracker.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Gui.h"
#include "../EngineClasses/Lt_Console.h"
#include "Bootstrapper.h"
#include "../Editor/Viewport.h"
#include "../Editor/NodeSys.h"
#include "../Editor/EditorViewport.h"
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
		uint32_t mainWindowIndex = 0;
		Lt_Vulkan vulkanHandler{};
		std::vector<std::function<void()>> UiUpdateFuncs;
	private:
		//Lt_Window ltWindow{ 800, 600 ,"LiteEngine : Agstrum"};
		FileLoader fileLoader{};
		Lt_WindowTracker windowMgr{};
		Lt_Gui guiHandler{};
		Viewport viewport{};
		EditorViewport editorViewport{};
		NodeSystem nodeSystem{nullptr};
		uint8_t frames = 0;
		
		bool mainResized = false;
	};
}