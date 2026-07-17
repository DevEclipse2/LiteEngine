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
//#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define MAX_BONE_PER_VERTEX 4
namespace lte {

	struct Lt_Scene
	{


	};

	struct Lt_VertexBoneData {
		uint32_t BoneIDs[MAX_BONE_PER_VERTEX] = {0};
		float   Weights[MAX_BONE_PER_VERTEX] = {0.0f};

		Lt_VertexBoneData()
		{
		}

		void AddBoneData(uint32_t BoneID, float Weight)
		{
			for (uint32_t i = 0; i < sizeof(BoneIDs)/sizeof(BoneIDs[0]); i++) 
			{
				if (Weights[i] == 0.0) {
					BoneIDs[i] = BoneID;
					Weights[i] = Weight;
					printf("bone %d weight %f index %i\n", BoneID, Weight, i);
					return;
				}
			}

			// should never get here - more bones than we have space for
			assert(0);
		}

	};
	class Lt_Importer
	{


		public:

		inline static std::vector<Lt_VertexBoneData> vertex_to_bones;
		inline static std::vector<int> mesh_base_vertex;
		inline static std::map<std::string, uint32_t> bone_name_to_index_map;


		static int get_bone_id(const aiBone* pBone);
			

		static void parse_single_bone(int mesh_index, const aiBone* pBone);
		


		static void parse_mesh_bones(int mesh_index, const aiMesh* pMesh);

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