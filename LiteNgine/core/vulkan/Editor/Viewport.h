#pragma once
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/FileLoader.h"
#include "../EngineClasses/Lt_Vulkan.h"

namespace lte {
	class Viewport
	{
	public:
		void SubmitGUICommands();
		void Init();
	private:
		float f = 0;

		VkDescriptorSet DS;
		LtImage image{};
		uint32_t viewportImageIndex = 0;
	};	

}