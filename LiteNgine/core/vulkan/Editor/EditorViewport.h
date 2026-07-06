#pragma once
#include "../Reworked/SwapchainHandler.h"
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/LtSync.h"
#include "../Reworked/DeviceHandler.h"
#include "../Reworked/TemporaryDraw.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Console.h"

namespace lte {
	class EditorViewport
	{
	public:

		uint8_t swapFrame = 0;

		//create swapchain, lag behind 1 frame
		void Init(ImVec2 size, uint8_t FramesInFlight);
		void Recreate(ImVec2 size , uint8_t FRamesInFlight);
		void UpdateGui();
		void RenderScene();
		void FinishFrame();
		EditorViewport();
		~EditorViewport();
		uint8_t framesInFlight = 2;

	private:
		ImVec2 size = ImVec2(800, 600);
		uint32_t frameNum = 0;
		vk::raii::DescriptorPool descriptorPool = nullptr;
		std::vector<std::unique_ptr<LtImage>> images;
		std::vector<VkDescriptorSet> descriptorSets;
		LtPipeline pipeline;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		LtImage depthImage{};

		std::vector<LtMeshInfo> meshes;
		std::vector<RenderSet> renderSets;
		vk::raii::Sampler sampler = nullptr;
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::Buffer indexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		vk::raii::DeviceMemory indexBufferMemory = nullptr;

		void createImages();// depth color and 2 out images
		void createPipeline();//idk something here
		void SubmitCommands(vk::raii::CommandBuffer& commandBuffer);//maybe reference simpledraw
		void UpdateUniformBuffers();
		/*void recreateImages();
		void createSyncSets();*/
		LtSyncSet syncSet{};
		//??????
		float fps = 1.0f;
		float prevtime;
		float frameTime;

	};
}

