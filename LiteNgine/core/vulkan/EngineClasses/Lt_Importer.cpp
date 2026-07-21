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
	

	uint8_t Lt_Importer::ParseMesh(aiMesh* mesh, Lt_MeshData& data)
	{
		data.vertexBuffer.reserve(mesh->mNumVertices);

		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex v;

			v.pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
			if (mesh->HasNormals()) {
				v.normal.x = mesh->mNormals[i].x;
				v.normal.y = mesh->mNormals[i].y;
				v.normal.z = mesh->mNormals[i].z;
			}
			if (mesh->mTextureCoords[0]) {

				//up to 8 uv channels , use uv 0
				v.texCoord.x = mesh->mTextureCoords[0][i].x;
				v.texCoord.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				v.texCoord = { 0.0f, 0.0f };
			}
			data.vertexBuffer.push_back(v);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++) {
				data.indexBuffer.push_back(face.mIndices[j]);
			}
		}
		return 0;
	}

	uint8_t Lt_Importer::ParseSkinnedMesh(aiMesh* mesh, Lt_SkinnedMeshData& skinnedmeshData)
	{

		Lt_MeshData baseData;		
		ParseMesh(mesh, baseData);
		skinnedmeshData.indexBuffer = std::move(baseData.indexBuffer); // borrow (steal) the indices

		skinnedmeshData.skinnedVertexBuffer.reserve(baseData.vertexBuffer.size());
		skinnedmeshData.WeightedVertexBuffer.resize(baseData.vertexBuffer.size());
		//big resize reduces memory operations 

		for (const Vertex& v : baseData.vertexBuffer) {
			skinnedmeshData.skinnedVertexBuffer.push_back(v);
		}

		//bone shi


		for (unsigned int i = 0; i < mesh->mNumBones; i++)
		{
			aiBone* bone = mesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			uint8_t boneID;

			if (m_currentSkinnedModel.BoneIndexes.find(boneName) == m_currentSkinnedModel.BoneIndexes.end())
			{
				boneID = static_cast<uint8_t>(m_currentSkinnedModel.bones.size());
				m_currentSkinnedModel.BoneIndexes[boneName] = boneID;
				Bone newBone;
				m_currentSkinnedModel.bones.push_back(newBone);
			}
			else {
				boneID = m_currentSkinnedModel.BoneIndexes[boneName];
			}

			// 3. Apply weights to the vertices
			for (unsigned int w = 0; w < bone->mNumWeights; w++)
			{
				int vertexId = bone->mWeights[w].mVertexId;
				float weight = bone->mWeights[w].mWeight;

				std::pair<float, uint8_t> weightBonePair = std::pair<float,uint8_t>(weight,boneID);
				skinnedmeshData.WeightedVertexBuffer[vertexId].SubmitWeight(weightBonePair);
			}
		}

		for (int i = 0; i < skinnedmeshData.skinnedVertexBuffer.size(); i++)
		{
			skinnedmeshData.WeightedVertexBuffer[i].ResolveWeights(skinnedmeshData.skinnedVertexBuffer[i]);
		}
		skinnedmeshData.WeightedVertexBuffer.clear();
		skinnedmeshData.WeightedVertexBuffer.shrink_to_fit();
		return 0;
	}

	void Lt_Importer::ParseNode(aiNode* node, const aiScene* scene)
	{
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			if (mesh->HasBones()) {
				Lt_SkinnedMeshData data; 
				ParseSkinnedMesh(mesh, data);
				m_currentSkinnedModel.subMeshes.push_back(data);
			}
			else
			{
				Lt_MeshData data;
				ParseMesh(mesh, data);
				m_currentStaticModel.subMeshes.push_back(data);
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ParseNode(node->mChildren[i], scene);
		}
	}
	uint8_t Lt_Importer::ParseScene(const aiScene* pScene , Lt_Scene& scene)
	{
		printf("*******************************************************\n");
		printf("Parsing %d meshes\n\n", pScene->mNumMeshes);
		int totalVertices = 0,totalIndices = 0,totalBones = 0;

		for (unsigned int i = 0; i < pScene->mNumMeshes; i++) {
			const aiMesh* pMesh = pScene->mMeshes[i];
			aiNode* rootNode = pScene->mRootNode;
			bool IsSkinned = pScene->HasSkeletons();
			int num_vertices = pMesh->mNumVertices;
			int num_indices = pMesh->mNumFaces * 3;
			int num_bones = pMesh->mNumBones;
			ParseNode(rootNode, pScene);
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