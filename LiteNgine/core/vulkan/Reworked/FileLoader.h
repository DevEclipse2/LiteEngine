#pragma once
#include <vulkan/vulkan_raii.hpp>
#include <stb_image.h>
#include "ImageDelegate.h"
#include "Buffers.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include <tiny_obj_loader.h>
#include "LtMesh.h"
//#include <string>
namespace lte 
{
	class FileLoader
	{

		public:
			static void createTextureImage(std::string path, uint32_t ImageIndex, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice, singleTimeCommandInfo cmdInfo);

			static void TemporaryFileLoad(vk::raii::Device* device, vk::raii::PhysicalDevice* physDevice, singleTimeCommandInfo info);

			static uint32_t objectCount;
			static std::vector<uint32_t> imageIndexes;
			static void loadModel(std::vector<Vertex>* pVertices, std::vector<uint32_t>* pIndices, std::string path);

			static Vertex* VertexArray;
			static uint32_t VertexesSize;
			static uint32_t* IndicesArray;
			static uint32_t IndicesSize;
			static std::vector<uint32_t> VertexSizes;
			static std::vector<uint32_t> IndiceSizes;
			static std::vector<RenderSet> renderSets;

		private:
			static std::vector<std::vector<Vertex>> vertexBuf;
			static std::vector<std::vector<uint32_t>> indexBuf;
	};

	

}
