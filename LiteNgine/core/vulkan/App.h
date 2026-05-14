#pragma once
#include <GLFW/glfw3.h>
/*
#include "VulkanDevice.h"
#include "../../core/engine/ImGuiBackend.h"
#include "../../core/engine/Ui/UiLayout.h"*/
#include "InterfaceLayers/Lt_ILayer.h"
#include "EngineClasses/Lt_WindowTracker.h"
#include "EngineClasses/Lt_MultiWindow.h"
namespace lte 
{
	class main {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		//NAME = "LiteEngine: Agstrum";
		void run();
	private:
		Lt_ILayer InterfaceLayer{};
		Lt_MultiWindow* mainWindow = nullptr;
	};
}
