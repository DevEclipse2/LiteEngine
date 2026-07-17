#include "Lt_Importer.h"
#define Load_Success 0
#define Load_Fail_Generic 1
#define Load_Fail_UnsupportedFile	2
#define Load_Fail_UnparsableFormat	3
#define Load_Fail_CorruptedFile		4

namespace lte 
{
	uint8_t Lt_Importer::Load(const std::string& path, unsigned int pFlags)
	{
		if (pFlags == 0) 
		{
			//no flags provided
			pFlags = aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType;
		}
		Assimp::Importer importer;
		const aiScene* importedScene = importer.ReadFile(path,pFlags);
		if (importedScene == nullptr) 
		{
			std::string errstr = importer.GetErrorString();
			Con::LogFailure(errstr + "Assimp loading of model failed from internal engine class Lt_Importer", HIGH_SEVERITY,TAG_ENGINE);
			return Load_Fail_Generic;
		}
		Lt_Scene scene{};
		ParseScene(importedScene,scene);
		return 0;

	}
	int Lt_Importer::get_bone_id(const aiBone* pBone)
	{
		{
			int bone_id = 0;
			std::string bone_name(pBone->mName.C_Str());

			if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end()) {
				// Allocate an index for a new bone
				bone_id = (int)bone_name_to_index_map.size();
				bone_name_to_index_map[bone_name] = bone_id;
			}
			else {
				bone_id = bone_name_to_index_map[bone_name];
			}

			return bone_id;
		}
	}
	void Lt_Importer::parse_single_bone(int mesh_index, const aiBone* pBone)
	{
		printf("      Bone '%s': num vertices affected by this bone: %d\n", pBone->mName.C_Str(), pBone->mNumWeights);

		int bone_id = get_bone_id(pBone);
		printf("bone id %d\n", bone_id);

		for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
			if (i == 0) printf("\n");
			const aiVertexWeight& vw = pBone->mWeights[i];

			uint32_t global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;
			printf("Vertex id %d ", global_vertex_id);

			assert(global_vertex_id < vertex_to_bones.size());
			vertex_to_bones[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
		}

		printf("\n");
	}

	void Lt_Importer::parse_mesh_bones(int mesh_index,const aiMesh* pMesh)
	{
		for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
			parse_single_bone(mesh_index, pMesh->mBones[i]);
		}
	}
	

	uint8_t Lt_Importer::ParseScene(const aiScene* pScene , Lt_Scene& scene)
	{
		printf("*******************************************************\n");
		printf("Parsing %d meshes\n\n", pScene->mNumMeshes);
		int totalVertices = 0,totalIndices = 0,totalBones = 0;

		for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
			const aiMesh* pMesh = pScene->mMeshes[i];
			int num_vertices = pMesh->mNumVertices;
			int num_indices = pMesh->mNumFaces * 3;
			int num_bones = pMesh->mNumBones;
			//printf("  Mesh %d '%s': vertices %d indices %d bones %d\n\n", i, pMesh->mName.C_Str(), num_vertices, num_indices, num_bones);
			totalVertices += num_vertices;
			totalIndices += num_indices;
			totalBones += num_bones;

			if (pMesh->HasBones()) {
				parse_mesh_bones(i,pMesh);
			}

			//printf("\n");
		}

		//printf("\nTotal vertices %d total indices %d total bones %d\n", totalVertices, totalIndices, totalBones);

		return 0;
	}
}