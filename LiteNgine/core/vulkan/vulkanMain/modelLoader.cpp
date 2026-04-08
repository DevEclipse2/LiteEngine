#include "../VulkanDevice.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include <tiny_obj_loader.h>

namespace lte {
	void VulkanDevice::loadModel(uint32_t index, std::string path)
	{

		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      warn, err;
		std::vector<Vertex>* pVertices = &vertices[index];
		std::vector<uint32_t>* pIndices = &indices[index];
		/*bool LoadObj(attrib_t * attrib, std::vector<shape_t> *shapes, std::vector<material_t> *materials, std::string * err, const char* filename, const char* mtl_basedir = NULL,bool triangulate = true);*/

		std::cout << "loading!" << path.c_str() << "\n";
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
		{
			std::cout << (warn + err) << "\n";
		}
		std::cout << "loading complete! \n";


		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};

				vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2] };

				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1] }; // throws index out of range exception

				vertex.color = { 1.0f, 1.0f, 1.0f };

				if (!uniqueVertices.contains(vertex))
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(pVertices->size());
					pVertices->push_back(vertex);
				}

				pIndices->push_back(uniqueVertices[vertex]);
			}
		}
 		std::cout << "indexing complete! \n";
	}
}