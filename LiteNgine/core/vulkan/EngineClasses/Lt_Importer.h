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

//asset importer shennanigans
namespace lte {

	struct Lt_Scene
	{


	};

	
	class Lt_Importer
	{
		struct Lt_MeshData
		{
			std::vector<Vertex> vertexBuffer;
			std::vector<uint32_t> indexBuffer;
		};
		struct Lt_SkinnedMeshData
		{
			std::vector<lte::SkinnedProcessorVertex> WeightedVertexBuffer;
			std::vector<skinnedVertex> skinnedVertexBuffer;
			std::vector<uint32_t> indexBuffer;
		};

		struct StaticModel {
			std::vector<Lt_MeshData> subMeshes;
		};
		struct SkinnedModel
		{
			std::vector<Lt_SkinnedMeshData> subMeshes;
			std::unordered_map<std::string, uint8_t> BoneIndexes;
			std::vector<lte::Bone> bones;
		};
	public:

		static uint8_t ParseMesh(aiMesh* mesh,Lt_MeshData& data);
		static uint8_t ParseSkinnedMesh(aiMesh* mesh,Lt_SkinnedMeshData& data);
		static void ParseNode(aiNode* node, const aiScene* scene);

		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		//static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();
		static uint8_t ParseScene(const aiScene* pScene, Lt_Scene& scene);

	private:
		//std::unordered_map<uint16_t, std::vector<Vertex>> vtx;
		inline static std::vector<StaticModel> staticModels;
		inline static std::vector<SkinnedModel> skinnedModels;

		inline static StaticModel m_currentStaticModel;
		inline static SkinnedModel m_currentSkinnedModel;
	};
}