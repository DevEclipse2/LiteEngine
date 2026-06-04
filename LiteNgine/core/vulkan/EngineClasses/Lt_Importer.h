#pragma once
#include <iostream>
#include <string>
#include <vector>
// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../Reworked/ltMesh.h"
#include "../EngineClasses/Lt_Console.h"
//asset importer shennanigans
namespace lte {
	class Lt_Importer
	{
		public:


		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();

	};
}