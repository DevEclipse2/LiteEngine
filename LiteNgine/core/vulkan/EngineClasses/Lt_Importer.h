#pragma once
#include <iostream>
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

		inline glm::mat4 convertAssimpMatrix(const aiMatrix4x4& from) {
			glm::mat4 to;
			to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
			to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
			to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
			to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
			return to;
		}
		
		static uint8_t Load(const std::string& path,unsigned int pFlags); //loads model
		//static unsigned int GetPreset(uint8_t presets);
		//uint8_t CreateIndexFile();
		//uint8_t Unpack();
		static uint8_t ParseScene(const aiScene* pScene, Lt_Scene& scene);
	};
}