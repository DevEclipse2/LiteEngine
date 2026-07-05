#pragma once
#include "../Reworked/SwapchainHandler.h"
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/LtSync.h"
#include "../Reworked/DeviceHandler.h"
#include "../Reworked/TemporaryDraw.h"
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
		void UpdateGui();
		EditorViewport();
		~EditorViewport();

	private:
		ImVec2 size = ImVec2(800, 600);
		uint8_t framesInFlight = 2;
		uint32_t frameNum = 0;
		uint8_t swapFrame = 0;
		std::vector<std::unique_ptr<LtImage>> images;
		std::vector<VkDescriptorSet> descriptorSets;
		LtPipeline pipeline;
		std::vector<vk::raii::CommandBuffer> commandBuffers;
		LtImage colorImage{};
		LtImage depthImage{};

		std::vector<LtMeshInfo> meshes;
		vk::raii::Buffer* vertexBuf = nullptr;
		vk::raii::Buffer* indexBuf = nullptr;
		std::vector<RenderSet> renderSets;
		vk::raii::Sampler sampler = nullptr;
		vk::raii::DescriptorPool descriptorPool = nullptr;
		vk::raii::Buffer vertexBuffer = nullptr;
		vk::raii::Buffer indexBuffer = nullptr;
		vk::raii::DeviceMemory vertexBufferMemory = nullptr;
		vk::raii::DeviceMemory indexBufferMemory = nullptr;

		void createImages();// depth color and 2 out images
		void createPipeline();//idk something here
		void SubmitCommands(vk::raii::CommandBuffer& commandBuffer);//maybe reference simpledraw
		void UpdateUniformBuffers( );
		/*void recreateImages();
		void createSyncSets();*/
		
		LtSyncSet syncSet{};
		void CreateImGuiDescriptionSets(VkDescriptorSet& ds, LtImage& img);
		//??????
		float fps = 1.0f;
		float prevtime;
		float frameTime;

	};
}

