#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_raii.hpp>
#include<array>
#include<functional>

namespace lte {
	
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}
		static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() {
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord))
			};
		}

		bool operator==(const Vertex &other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	class VertexHandler
	{
	public:

	//	const std::vector<Vertex> vertices = {
	//{{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	//{{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	//{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	//{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	//{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},           
	//{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	//	};

	//	
	//	const std::vector<uint16_t> indices = {
	//		//0, 1, 2, 2, 3, 0 , 2, 1 , 0 , 0, 3 ,2,
	//		0, 1, 2, 2, 3, 0,
	//		4, 5, 6, 6, 7, 4,
	//		
	//		0, 1, 2, 2, 3, 0,
	//		4, 7, 6, 6, 5, 4,
	//		1, 5, 6, 6, 2, 1,
	//		//1, 2, 6, 6, 5, 1,
	//		2, 6, 7, 7, 3, 2,
	//		//2, 3, 7, 7, 6, 2,
	//		5, 4, 7, 7, 6, 5,
	//		//5, 6, 7, 7, 4, 5,
	//		3, 7, 4, 4, 0, 3,
	//		1, 5, 4, 4, 0, 1,
	//		1, 0, 4, 4, 5, 1
	//	};
	};
}


namespace std {
	template<> struct hash<lte::Vertex> {
		size_t operator()(lte::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}