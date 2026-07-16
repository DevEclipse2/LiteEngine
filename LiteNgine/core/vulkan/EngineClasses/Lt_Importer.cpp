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
	void parse_single_bone(int bone_index, const aiBone* pBone)
	{
		//printf("      Bone %d: '%s' num vertices affected by this bone: %d\n", bone_index, pBone->mName.C_Str(), pBone->mNumWeights);

		for (unsigned int i = 0; i < pBone->mNumWeights; i++) {
			if (i == 0) printf("\n");
			const aiVertexWeight& vw = pBone->mWeights[i];
			//printf("       %d: vertex id %d weight %.2f\n", i, vw.mVertexId, vw.mWeight);
		}

		//printf("\n");
	}


	void parse_mesh_bones(const aiMesh* pMesh)
	{
		for (unsigned int i = 0; i < pMesh->mNumBones; i++) {
			parse_single_bone(i, pMesh->mBones[i]);
		}
	}

	uint8_t Lt_Importer::ParseScene(const aiScene* pScene , Lt_Scene& scene)
	{
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
				parse_mesh_bones(pMesh);
			}

			//printf("\n");
		}

		//printf("\nTotal vertices %d total indices %d total bones %d\n", totalVertices, totalIndices, totalBones);

		return 0;
	}
}