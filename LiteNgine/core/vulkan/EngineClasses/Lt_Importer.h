#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>
// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include "../Reworked/ltMesh.h"
#include "../EngineClasses/Lt_Console.h"
#include "../Reworked/CommandBuffers.h"
//asset importer shennanigans
namespace lte {

	class Lt_Importer
	{

	public:
		struct SkeletonNode {
			std::string name;
			glm::mat4 defaultLocalTransform;
			std::vector<SkeletonNode> children;
		};
		struct Lt_Material {
			uint32_t diffuseTextureIndex = -1; // in
			uint32_t normalTextureIndex = -1;
		};

		struct StrippedModel// model with only relevant transforms and stuff
		{
			SkeletonNode rootNode;
			std::vector<RenderSet> staticRenderset;
			std::vector<RenderSet> skinnedRenderset;
			glm::mat4 transform;
			std::vector<glm::mat4> transforms;
			std::vector<glm::mat4> skinnedTransforms;
			std::unordered_map<std::string, uint16_t> BoneIndexes;
			std::vector<Bone> bones;
			std::string name;
		};

		struct Lt_MeshData
		{
			std::vector<Vertex> vertexBuffer;
			std::vector<uint32_t> indexBuffer;
			uint32_t VertexCount;
			uint32_t IndexCount;
			uint32_t materialIndex = -1;
		};
		struct Lt_SkinnedMeshData
		{
			std::vector<SkinnedProcessorVertex> WeightedVertexBuffer;
			std::vector<skinnedVertex> skinnedVertexBuffer;
			std::vector<uint32_t> indexBuffer;
			uint32_t VertexCount;
			uint32_t IndexCount;
			uint32_t materialIndex = -1;
			std::vector<uint16_t> bonePalette;
		};

		struct Model {
			glm::mat4 transform;
			SkeletonNode rootNode;
			std::vector<Lt_MeshData> subMeshes;
			std::vector<Lt_SkinnedMeshData> skinnedSubMeshes;
			std::vector<glm::mat4> transforms;
			std::vector<glm::mat4> skinnedTransforms;
			std::unordered_map<std::string, uint16_t> BoneIndexes;
			std::vector<Bone> bones;
			uint32_t VertexCount;
			uint32_t IndexCount;
			std::string name;
		};
		struct PositionKey {
			glm::vec3 position;
			float timeStamp;
		};

		struct RotationKey {
			glm::quat orientation;
			float timeStamp;
		};

		struct ScaleKey {
			glm::vec3 scale;
			float timeStamp;
		};

		// All keyframes for a single bone
		struct BoneTransformTrack {
			std::string boneName;
			std::vector<PositionKey> positions;
			std::vector<RotationKey> rotations;
			std::vector<ScaleKey> scales;
		};
		struct Animation {
			std::string name;
			float duration;
			float ticksPerSecond;
			std::vector<BoneTransformTrack> boneTracks;
		};
	public:
		static inline glm::mat4 ConvertAssimpMatrixToGLM(const aiMatrix4x4& from) {
			glm::mat4 to;
			// Transposing from row-major to column-major
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}
		static uint8_t ParseMesh(aiMesh* mesh,Lt_MeshData& data);
		static uint8_t ParseSkinnedMesh(aiMesh* mesh, std::vector<Lt_SkinnedMeshData>& outSubMeshes,Model& model );
		static void ParseNode(aiNode* node, const aiScene* scene, Model& currentModel, glm::mat4 parentTransform);
		static void ParseNodeHierarchy(const aiNode* node, SkeletonNode& engineNode);
		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		//static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();
		static uint8_t ParseScene(const aiScene* pScene, const std::string& directory);
		static void LoadAnimation(const aiScene* scene, Animation& outAnimation, int animIndex = 0);
		static uint8_t GenerateRenderSets(singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice);
		static uint8_t RemoveModels();
		inline static std::vector<RenderSet> renderSets;
		inline static std::vector<StrippedModel> strippedModels;
		static int GetPositionIndex(float animationTime, const BoneTransformTrack& track);
		static int GetRotationIndex(float animationTime, const BoneTransformTrack& track);
		static int GetScaleIndex(float animationTime, const BoneTransformTrack& track);
		static glm::vec3 InterpolatePosition(float animationTime, const BoneTransformTrack& track);
		static glm::quat InterpolateRotation(float animationTime, const BoneTransformTrack& track);
		static glm::vec3 InterpolateScale(float animationTime, const BoneTransformTrack& track);
		static std::string ResolveTexturePath(const std::string& assimpPathStr);
		inline static uint32_t fallBackImageIndex =-1;
		inline static Animation animation;
		static void UpdateHierarchy(const SkeletonNode& node, const glm::mat4& parentTransform, float animationTime, StrippedModel& model, LtSkinnedMeshInfo& meshInfo, const Animation& animation);
		static const BoneTransformTrack* FindBoneTrack(const Animation& animation, const std::string& nodeName);
		static std::string RemovePrefix(std::string name, std::vector<std::string>& filterWords);

	private:
		//std::unordered_map<uint16_t, std::vector<Vertex>> vtx;
		inline static std::vector<Model> loadedModels;
		inline static Model m_currentStaticModel;
		inline static uint32_t totalVertices;
		inline static uint32_t totalIndices;
		inline static std::vector<Lt_Material> sceneMaterials;
		inline static std::unordered_map<std::string, int> loadedTextureMap;
		inline static uint32_t renderSetOffset = 0;
		static bool HasSkinnedMeshes(const aiNode* node, const aiScene* scene);
	};
}