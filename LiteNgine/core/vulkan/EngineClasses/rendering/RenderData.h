#pragma once
#include "../../Reworked/LtMesh.h"
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

		//render sets

		inline static std::vector<RenderSet> renderSets;
		inline static std::vector<LtMeshInfo> MeshInformation;
		void FillBuffer()
		{
			
		}
	};
}

