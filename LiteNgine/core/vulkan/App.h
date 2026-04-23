#pragma once
#include <GLFW/glfw3.h>
#include "Lt_Window.h"
/*
#include "VulkanDevice.h"
#include "../../core/engine/ImGuiBackend.h"
#include "../../core/engine/Ui/UiLayout.h"*/
#include "InterfaceLayers/Lt_ILayer.h"

namespace lte 
{
	class main {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		//NAME = "LiteEngine: Agstrum";
		void run();
	private:
		Lt_Window* pWindow = nullptr;
		Lt_ILayer InterfaceLayer{};
		/*
		VulkanDevice vkDevice	{ ptr };
		Gui uiProc {&vkDevice};
		UiLayout uiLayout{};*/
	};
}
