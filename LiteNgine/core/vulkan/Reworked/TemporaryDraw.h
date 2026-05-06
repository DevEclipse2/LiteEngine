#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <chrono>
#include "SwapchainHandler.h"
#include "LtMesh.h"
#include "ImageDelegate.h"
#include "FileLoader.h"
namespace lte {
	class TemporaryDraw
	{
	public:
		static float prevtime;
		static float fps;
		static float frameTime;
		static void updateUniformBuffer(uint32_t frame, LtSwapChain& swap, std::vector<LtMeshInfo>& info);
		static void recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex, vk::raii::CommandBuffer& commandBuffer, LtSwapChain* swapChainImage,
			LtImage& colorImage, LtImage& depthImage, LtPipeline* pipeline, vk::raii::Buffer* vertexBuf, vk::raii::Buffer* indexBuf,
			vk::raii::DeviceMemory* vertexMem, vk::raii::DeviceMemory* indiceMem, std::vector<LtMeshInfo>* meshes, std::vector<RenderSet>* rendersets);

	};
}