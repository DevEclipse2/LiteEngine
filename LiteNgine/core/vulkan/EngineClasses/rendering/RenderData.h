#pragma once
#include "../../Reworked/CommandBuffers.h"
#include "../../Reworked/LtMesh.h"
#include "vk_mem_alloc.h"
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
			SkinnedVertexBuffer,
			XLVertexBuffer, // for special operations 
			XLSkinnedVertexBuffer,
			XLIndexBuffer,

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
		inline static std::vector<std::unique_ptr<Buffer>> Buffers;
		enum copyResult {
			Sucess,
			FailureGeneric,
			FailureExceedBufferSize
		};
		enum freeResult {
			Sucess,
			FailureGeneric,
			FailureIncompatibleType,
		};
		static void createBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device,BufferType type);
		static copyResult copyBufferContents(std::vector<Vertex> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, RenderSet& renderset);
		static copyResult createXLVertexBuffer(std::vector<Vertex> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, RenderSet& renderset); //for those truly whale sized models
		static copyResult copyVertexBufferContentsBulk(std::vector<std::vector<Vertex>> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, std::vector<RenderSet&> rendersets);
		static void DefragmentBuffer(uint16_t BufferId,uint32_t* savings);//defragmentation
		static void MarkFreedVertexes(RenderSet& renderset);
		inline static std::vector<RenderSet> renderSets;

		inline static std::vector<LtMeshInfo> MeshInformation;
		void FillBuffer()
		{
			
		}
	};
}

