#pragma once
#include "../Reworked/SwapchainHandler.h"
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/LtSync.h"
#include "../Reworked/DeviceHandler.h"
#include "../Reworked/TemporaryDraw.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Console.h"
#include "ViewportCamera.h"
#include "LtUiWindow.h"
#include "../EngineClasses/Assimp/Lt_Importer.h"

namespace lte {
	class EditorViewport : LtUiWindow
	{
	public:

		uint8_t swapFrame = 0;

		//create swapchain, lag behind 1 frame
		void Init(ImVec2 size, uint8_t FramesInFlight);
		void Recreate(ImVec2 size , uint8_t FRamesInFlight);
		void SubmitGUICommands() override;
		void RenderScene(vk::raii::Semaphore& semaphore);
		void FinishFrame();
		EditorViewport();
		~EditorViewport();
		uint8_t framesInFlight = 2;
		LtSyncSet syncSet{};
		LtImage depthImage{};
		LtImage colorImage{};

		std::vector<LtMeshInfo> meshes = {};
	private:
		ImVec2 size = ImVec2(800, 600);
		uint32_t frameNum = 0;
		std::vector<std::unique_ptr<LtImage>> images = {};
		vk::raii::DescriptorPool descriptorPool = nullptr;

		std::vector<VkDescriptorSet> descriptorSets = {};

		vk::raii::DescriptorPool dynamicDescriptorPool = nullptr;

		std::vector<vk::raii::DescriptorSet> dynamicDescriptorSets = {};

		LtPipeline pipeline;
		LtPipeline SkinnedPipeline;
		std::vector<vk::raii::CommandBuffer> commandBuffers = {};
		
		std::vector<LtSkinnedMeshInfo> skinnedMeshes = {};
		std::vector<RenderSet> renderSets = {};
		vk::raii::Sampler sampler = nullptr;
		void createImages();// depth color and 2 out images
		void createPipeline();//idk something here
		void SubmitCommands(vk::raii::CommandBuffer& commandBuffer);//maybe reference simpledraw
		void UpdateUniformBuffers();
		void RenderSkinnedMeshes(std::vector<LtSkinnedMeshInfo>& activeMeshes, const std::vector<Lt_Importer::StrippedModel>& characterAsset, vk::raii::CommandBuffer& cmdBuffer);
		/*void recreateImages();
		void createSyncSets();*/
		//??????
		float fps = 1.0f;
		float prevtime;
		float frameTime;
		float scale = 1.0f;
		int newHeight = 800;
		int newWidth = 600;
		int newFIF = 3;

		float x = 0;
		float y = 0;
		float z = 0;

		float rotX, rotY, rotZ = 0;
		float FOV = 45;

		ViewportCamera camera;
		bool isDragging = false;
		ImVec2 lastMousePos;

		
		
		//this is for skinned vertexes
		std::vector<vk::raii::Buffer> dynamicSkinnedUBO = {};
		std::vector<vk::raii::DeviceMemory> dynamicSkinnedMemory = {};
		std::vector<void*> dynamicUBOMappedPtr;

		std::vector <Lt_Importer::StrippedModel> skinnedModels = {};
		vk::raii::DescriptorSet skinnedDescriptorSet = nullptr;

		size_t dynamicAlignment;

		float animPlayHead = 0;

		int objectID		= 0;
		int SelectedObject	= -1;

	};
}

