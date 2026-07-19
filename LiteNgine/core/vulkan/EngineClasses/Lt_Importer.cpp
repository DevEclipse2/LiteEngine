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
	

	void Lt_Importer::ParseNode(aiNode* node, const aiScene* scene)
	{
		// Process all the meshes attached to this specific node
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			// The node only contains an index to the actual mesh object in the scene
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			//meshes.push_back(ProcessMesh(mesh, scene));
			//ParseMesh(mesh);
		}

		// Recursively process all children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			//ProcessNode(node->mChildren[i], scene);
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


			//printf("\n");
		}

		//printf("\nTotal vertices %d total indices %d total bones %d\n", totalVertices, totalIndices, totalBones);

		return 0;
	}
}