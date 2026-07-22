#pragma once
#include "../../Reworked/CommandBuffers.h"
#include "../../Reworked/LtMesh.h"
#include "vk_mem_alloc.h"
#define MaxSkinnedVertexBuffer 2097152 // if a skinned vertex is 64 bytes this is 128mb. Perfect!
#define MaxVertexBuffer 3050402 // if a vertex is 44 bytes this is around 128mb. Perfect!
namespace lte {
	class RenderData
	{
	public:
		//static meshes
		inline static Vertex* VertexArray;
		inline static uint32_t VertexesSize;
		inline static uint32_t* IndicesArray;
		inline static uint32_t IndicesSize;
		//skinned meshes
		inline static skinnedVertex* skinnedVertexArray;
		inline static uint32_t skinnedVertexesSize;
		inline static uint32_t* skinnedIndicesArray;
		inline static uint32_t skinnedIndicesSize;

		//below are buffers to load into as the primary arrays should not be accessed by just any class

		inline static Vertex* VertexArrayBuffer;
		inline static uint32_t VertexesSizeBuffer;
		inline static uint32_t* IndicesArrayBuffer;
		inline static uint32_t IndicesSizeBuffer;
		
		inline static skinnedVertex* skinnedVertexArrayBuffer;
		inline static uint32_t skinnedVertexesSizeBuffer;
		inline static uint32_t* skinnedIndicesArrayBuffer;
		inline static uint32_t skinnedIndicesSizeBuffer;

		enum BufferType {
			GenericBuffer,
			VertexBuffer,
			IndiceBuffer,
			SkinnedVertexBuffer
		};

		struct Buffer
		{
			BufferType type = GenericBuffer;
			vk::raii::Buffer buffer = nullptr;
			std::vector<std::pair<uint32_t, uint32_t>> FreeSpace; // size, location ()
			vk::raii::DeviceMemory Allocation = nullptr;
			uint32_t offset = 0; // this tracks it in units such as vertex , NOT BYTES!
		};


		//render sets
		static void createVertexBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device);
		inline static std::vector<std::unique_ptr<Buffer>> Buffers;
		enum copyResult {
			Sucess,
			FailureGeneric,
			FailureExceedBufferSize
		};
		static copyResult copyVertexBufferContents(std::vector<Vertex> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, RenderSet& renderset);
		static copyResult copyVertexBufferContentsBulk(std::vector<std::vector<Vertex>> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, std::vector<RenderSet&> rendersets);

		static void MarkFreedVertexes();
		inline static std::vector<RenderSet> renderSets;

		inline static std::vector<LtMeshInfo> MeshInformation;
		void FillBuffer()
		{
			
		}
	};
}

