#pragma once
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/FileLoader.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Console.h"
namespace lte {
	class Viewport
	{
	public:
		void SubmitGUICommands();
		void Init();
		Viewport();
		~Viewport();
	private:
		float f = 0;

		GUI_Image image{};
		uint32_t viewportImageIndex = 0;
	};	

}