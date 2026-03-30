#include "VertexHandler.h"
namespace lte {
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec3 color;
		static vk::VertexInputBindingDescription getBindingDescription() {
			return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex };
		}
	};
}