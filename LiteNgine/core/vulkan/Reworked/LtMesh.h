#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <vulkan/vulkan_raii.hpp>
#include "../EngineClasses/Lt_Console.h"
#include "../EngineClasses/Lt_Importer.h"
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
		alignas(16) glm::mat4 boneTransforms[MAX_BONES]; 
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
	
	struct BoneInfo {
		int id;
		// The "Inverse Bind Pose" matrix. Converts vertices from model space into the bone's local space.
		glm::mat4 offset;
	};

	// Represents the skeleton hierarchy tree
	struct AssimpNodeData {
		std::string name;
		glm::mat4 transformation; // Node's default local transform
		std::vector<AssimpNodeData> children;
	};
	struct LtSkinnedMeshInfo : LtMeshInfo
	{	
		//skeletal data
		std::unordered_map<std::string, BoneInfo> boneInfoMap;
		int boneCounter = 0;
		AssimpNodeData rootNode;

		// Animation State
		float currentAnimationTime = 0.0f;
		//animations go here
		
		// Computes the boneTransforms array for the current frame
		void updateAnimation(float deltaTime, UniformBufferObject& ubo) {
			// Advance currentAnimationTime by deltaTime
			// Recursively traverse 'rootNode'
			// Interpolate translation/rotation/scale keyframes
			// Multiply child transform by parent transform
			// Store final matrix (GlobalTransform * OffsetMatrix) in ubo.boneTransforms[boneId]
		}
	};
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
		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

	struct SkinnedVertex : Vertex
	{
		glm::ivec4 boneIds;
		glm::vec4 boneWeights;
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
		bool operator==(const SkinnedVertex& other) const
		{
			return pos == other.pos && color == other.color && texCoord == other.texCoord && boneIds == other.boneIds && boneWeights == other.boneWeights;
		}
		void addBoneData(int boneId, float weight) {
			for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
				if (boneIds[i] < 0) {
					boneWeights[i] = weight;
					boneIds[i] = boneId;
					return;
				}
			}
			Con::LogError("more than 4 influences", MED_SEVERITY, TAG_ENGINE);
			// If it reaches here, a vertex has > 4 influences.
			// You'll need logic to keep the 4 highest weights and normalize them.
		}
	};

	

	struct RenderSet {
		//a render set contains all relevant data

		uint32_t vertexArrayStartIndex	= 0;
		uint32_t vertexArraySize		= 0;
		uint32_t IndiceArrayStartIndex	= 0;
		uint32_t IndiceArraySize		= 0;
		uint32_t imageIndex = 0;//might need more but its fine for now

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
	struct std::hash<lte::SkinnedVertex>
	{
		size_t operator()(lte::SkinnedVertex const& vertex) const noexcept
		{
			size_t seed = 0;

			// Helper lambda to combine hashes effectively to minimize collisions
			auto hashCombine = [&seed](size_t hashValue) {
				seed ^= hashValue + 0x9e3779b9 + (seed << 6) + (seed >> 2);
				};

			hashCombine(std::hash<glm::vec3>()(vertex.pos));
			hashCombine(std::hash<glm::vec3>()(vertex.color));
			hashCombine(std::hash<glm::vec2>()(vertex.texCoord));
			hashCombine(std::hash<glm::ivec4>()(vertex.boneIds));
			hashCombine(std::hash<glm::vec4>()(vertex.boneWeights));

			return seed;
		}
	};
}