#pragma once
#include "../Reworked/SwapchainHandler.h"
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/LtSync.h"
#include "../Reworked/DeviceHandler.h"
#include "../EngineClasses/Lt_Vulkan.h"
namespace lte {
	class EditorViewport
	{
	public:
		int FrameIndex = 0;
		int FrameAvailableIndex = -1;

		//create swapchain, lag behind 1 frame
		void Init(ImVec2 size, uint8_t FramesInFlight);
		void Recreate(ImVec2 size , uint8_t FRamesInFlight);
	private:
		ImVec2 size = ImVec2(800, 600);
		uint8_t framesInFlight = 2;
		std::vector<std::unique_ptr<LtImage>> images[2];
		LtImage colorImage{};
		LtImage depthImage{};
		void createImages();// depth color and 2 out images
		void createPipeline();//idk something here
		void SubmitCommands();//maybe reference simpledraw
		void UpdateUniformBuffers();
		void recreateImages();
		void createSyncSets();
	};
}

