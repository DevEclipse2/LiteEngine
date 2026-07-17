#pragma once
#include <iostream>
#include <map>
#include <string>
#include <vector>

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
	
		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		//static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();
		static uint8_t ParseScene(const aiScene* pScene, Lt_Scene& scene);
	};
}