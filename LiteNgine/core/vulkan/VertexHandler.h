#pragma once
#include <glm/glm.hpp>
#include<glm/vec2.hpp>
#include<glm/vec3.hpp>
#include <vulkan/vulkan_raii.hpp>
#include<array>

namespace lte {
	
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}
		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions()
		{
			return {
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color))
			};
		}
	};

	class VertexHandler
	{
	public:
		const std::vector<Vertex> vertices = {
		{{-0.5f	, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		const std::vector<uint16_t> indices = {
		0, 1, 2, 2, 3, 0
		};
	};
}


