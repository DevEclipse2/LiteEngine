#pragma once
#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include <vulkan/vulkan_raii.hpp>
#include<array>

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
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class VertexHandler
	{
	public:

		const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

	{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
	{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
	{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
	{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		};

		
		const std::vector<uint16_t> indices = {
			//0, 1, 2, 2, 3, 0 , 2, 1 , 0 , 0, 3 ,2,
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
			/*
			0, 1, 2, 2, 3, 0,
			4, 7, 6, 6, 5, 4,
			4, 5, 6, 6, 7, 4,
			1, 5, 6, 6, 2, 1,
			2, 6, 7, 7, 3, 2,
			5, 4, 7, 7, 6, 5,
			3, 7, 4, 4, 0, 3,
			1, 5, 4, 4, 0, 1
			*/
		};
	};
}


