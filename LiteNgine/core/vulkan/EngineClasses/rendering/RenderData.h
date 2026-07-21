#pragma once
#include "../../Reworked/CommandBuffers.h"
#include "../../Reworked/LtMesh.h"
#define MaxVertexBuffer 500000 // if a skinned vertex is 64 bytes this is 256mb Perfect!
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
			BufferType type;
			vk::raii::Buffer buffer = nullptr;
			vk::raii::DeviceMemory bufferMem = nullptr;
			uint32_t offset;
		};


		//render sets
		static void createVertexBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device);
		inline static std::vector<std::unique_ptr<Buffer>> Buffers;
		enum copyResult {
			Sucess,
			FailureGeneric,
			FailureExceedBufferSize
		};
		static copyResult copyVertexBufferContents(std::vector<Vertex> data);
		inline static std::vector<RenderSet> renderSets;
		inline static std::vector<LtMeshInfo> MeshInformation;
		void FillBuffer()
		{
			
		}
	};
}

