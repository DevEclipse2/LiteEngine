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


	public:

		static uint8_t ParseMesh(aiMesh* mesh);
		static uint8_t ParseSkinnedMesh(aiMesh* mesh);
		static void Lt_Importer::ParseNode(aiNode* node, const aiScene* scene);

		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		//static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();
		static uint8_t ParseScene(const aiScene* pScene, Lt_Scene& scene);

	private:
		std::unordered_map<uint16_t ,std::vector<Vertex>>
	};
}