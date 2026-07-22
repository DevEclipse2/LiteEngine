#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "../EngineClasses/Lt_Console.h"
#define MAX_BONES 128
#define MAX_BONE_INFLUENCE 4
namespace lte {
	
	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
	struct SkinnedUniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::mat4 boneTransforms[MAX_BONES] ;
		
	};
	enum class MeshType 
	{
		Static,
		Skinned
	};
	struct LtMeshInfo {
		
		// Transform properties
		glm::vec3 position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

		// Uniform buffer for this object (one per frame in flight)
		std::vector<vk::raii::Buffer> uniformBuffers;
		std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;

		// Descriptor sets for this object (one per frame in flight)
		std::vector<vk::raii::DescriptorSet> descriptorSets;

		// Calculate model matrix based on position, rotation, and scale
		glm::mat4 getModelMatrix() const {
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, position);
			model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::scale(model, -scale);
			return model;
		}
	};
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
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
		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};
	struct skinnedVertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::u8vec4 BoneIDs;
		glm::vec4 BoneWeights;

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			return { 0, sizeof(skinnedVertex), vk::VertexInputRate::eVertex };
		}
		static std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions() {
			return {
				vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(skinnedVertex, pos)),
				vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(skinnedVertex, color)),
				vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(skinnedVertex, texCoord)),
				vk::VertexInputAttributeDescription(3, 0, vk::Format::eR8G8B8A8Uint, offsetof(skinnedVertex, BoneIDs)),
				vk::VertexInputAttributeDescription(4, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(skinnedVertex, BoneWeights)),
			};
		}
		bool operator==(const skinnedVertex& other)  const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord && BoneIDs == other.BoneIDs && BoneWeights == other.BoneWeights;
		}
		
		skinnedVertex& operator=(const Vertex& other)
		{
			pos = other.pos;
			normal = other.normal;
			color = other.color;
			texCoord = other.texCoord;

			ResetBones();

			return *this;
		}
		skinnedVertex(const Vertex& other)
		{
			*this = other; // Reuse the assignment operator logic
		}
		void ResetBones() {
			for (int i = 0; i < 4; i++)
			{
				BoneIDs[i] = 0;
				BoneWeights[i] = 0;
			}
		}
	};

	struct SkinnedProcessorVertex {

		std::vector<std::pair<float, uint8_t>> WeightBonePair;
		void SubmitWeight(std::pair<float, uint8_t> weightbonePair)
		{
			WeightBonePair.emplace_back(weightbonePair);
		}

		void ResolveWeights(skinnedVertex& vertex)
		{
			std::sort(WeightBonePair.begin(), WeightBonePair.end());
			for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
			{
				vertex.BoneWeights[i] = WeightBonePair[i].first;
				vertex.BoneIDs[i] = WeightBonePair[i].second;
			}

			//drops everything
			WeightBonePair.clear();
			WeightBonePair.shrink_to_fit();
		}
	};

	struct Bone
	{
		glm::mat4 offsetMatrix;
		std::vector<uint8_t> children;//points to different boneids
	};
	

	struct LtSkinnedMeshInfo {
		std::vector<Bone> bones; // id is index number
		std::unordered_map<uint8_t, LtSkinnedMeshInfo> breakawayChildren;
		/// <summary>
		/// special feature : if the mesh exceeds 128 bones, the engine will split off into sub meshes
		/// </summary>
		uint8_t boneCounter;
	};

	
	
	
	struct RenderSet {
		//a render set contains all relevant data
		uint32_t vertexArrayStartIndex	= 0;
		uint32_t vertexArraySize		= 0;
		uint32_t IndiceArrayStartIndex	= 0;
		uint32_t IndiceArraySize		= 0;
		uint32_t imageIndex = 0;//might need more but its fine for now
		//uint8_t BufferID; //max is 256 because who the fuck has more vram than 
		//nvm its 16_t because 32 gb isnt really that much
		uint16_t bufferId = 0;
		MeshType type = MeshType::Static;

		RenderSet(uint32_t vSI,uint32_t vAS, uint32_t iSI, uint32_t iAS, uint32_t iI) : vertexArrayStartIndex{vSI} , vertexArraySize{vAS} , IndiceArrayStartIndex{iSI}, IndiceArraySize{iAS}, imageIndex{iI}
		{}
	};
	
	class LtMesh
	{

	};
}
namespace std {
	template <>
	struct std::hash<lte::Vertex>
	{
		size_t operator()(lte::Vertex const& vertex) const noexcept
		{
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
		}

	};

	template <>
	struct std::hash<lte::skinnedVertex>
	{
		size_t operator()(lte::skinnedVertex const& vertex) const noexcept
		{
			size_t seed = 0;

			// Helper lambda to combine hashes effectively to minimize collisions
			auto hashCombine = [&seed](size_t hashValue) {
				seed ^= hashValue + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				};

			hashCombine(std::hash<glm::vec3>()(vertex.pos));
			hashCombine(std::hash<glm::vec3>()(vertex.color));
			hashCombine(std::hash<glm::vec2>()(vertex.texCoord));
			hashCombine(std::hash<glm::u8vec4>()(vertex.BoneIDs));
			hashCombine(std::hash<glm::vec4>()(vertex.BoneWeights));

			return seed;
		}
	};
	
}