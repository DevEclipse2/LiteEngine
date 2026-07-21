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
		ParseScene(importedScene);
		return 0;

	}
	

	uint8_t Lt_Importer::ParseMesh(aiMesh* mesh, Lt_MeshData& data)
	{
		data.vertexBuffer.reserve(mesh->mNumVertices);

		data.VertexCount = mesh->mNumVertices;
		data.IndexCount = mesh->mNumFaces * 3;
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

	uint8_t Lt_Importer::ParseSkinnedMesh(aiMesh* mesh, Lt_SkinnedMeshData& skinnedmeshData, Model& model)
	{

		Lt_MeshData baseData;		
		ParseMesh(mesh, baseData);
		skinnedmeshData.indexBuffer = std::move(baseData.indexBuffer); // borrow (steal) the indices
		skinnedmeshData.VertexCount = mesh->mNumVertices;
		skinnedmeshData.IndexCount = mesh->mNumFaces * 3;
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

			if (model.BoneIndexes.find(boneName) == model.BoneIndexes.end())
			{
				boneID = static_cast<uint8_t>(model.bones.size());
				model.BoneIndexes[boneName] = boneID;
				Bone newBone;
				model.bones.push_back(newBone);
			}
			else {
				boneID = model.BoneIndexes[boneName];
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

	void Lt_Importer::ParseNode(aiNode* node, const aiScene* scene, Model& model, glm::mat4 parentTransform)
	{
		glm::mat4 nodeTransform = ConvertAssimpMatrixToGLM(node->mTransformation);
		glm::mat4 accumulatedTransform = parentTransform * nodeTransform;

		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

			if (mesh->HasBones()) {
				Lt_SkinnedMeshData data;
				model.skinnedTransforms.emplace_back(accumulatedTransform);
				ParseSkinnedMesh(mesh, data,model);
				model.skinnedSubMeshes.push_back(data);
				model.skinnedVertexCount += data.VertexCount;
				model.skinnedIndexCount += data.IndexCount;
			}
			else
			{
				model.transforms.emplace_back(accumulatedTransform);
				Lt_MeshData data;
				ParseMesh(mesh, data);
				model.subMeshes.push_back(data);
				model.IndexCount	+= data.IndexCount;
				model.VertexCount	+= data.IndexCount;

			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ParseNode(node->mChildren[i], scene,model,nodeTransform);
		}
	}

	uint8_t Lt_Importer::ParseScene(const aiScene* pScene)
	{
		printf("*******************************************************\n");
		printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

		aiNode* rootNode = pScene->mRootNode;

		glm::mat4 rootTransform = ConvertAssimpMatrixToGLM(rootNode->mTransformation);
		//ParseNode(rootNode, pScene);

		for (unsigned int i = 0; i < rootNode->mNumChildren; i++)
		{
			aiNode* topLevelNode = rootNode->mChildren[i];


			Model newObject;
			LtMeshInfo meshInfo;
			newObject.name = topLevelNode->mName.C_Str();
			glm::mat4 localTransform = ConvertAssimpMatrixToGLM(topLevelNode->mTransformation);
			newObject.transform = rootTransform * localTransform;

			ParseNode(topLevelNode, pScene, newObject,rootTransform);

			loadedModels.push_back(newObject);
		}

		//Vertex* newVertexes = new Vertex[totalVertices];
		//VertexArray = newVertexes;
		//uint32_t* newIndices = new uint32_t[totalIndices];
		//IndicesArray = newIndices;

		//uint32_t Vindexes = 0;
		//uint32_t Iindexes = 0;
		//for (uint32_t i = 0; i < vertexBuf.size(); i++)
		//{
		//	if (vertexBuf[i].size() == 0) continue;
		//	//starts at 0, length 12
		//	//renders from zero to 11
		//	//next one starts rendering from 12
		//	RenderSet rs{ Vindexes,static_cast<uint32_t>(vertexBuf[i].size()),Iindexes,static_cast<uint32_t>(indexBuf[i].size()),imageIndexes[i] };
		//	renderSets.emplace_back(rs);
		//	//so we was reading garbage this whole time
		//	memcpy(VertexArray + Vindexes, vertexBuf[i].data(), sizeof(Vertex) * vertexBuf[i].size());
		//	memcpy(IndicesArray + Iindexes, indexBuf[i].data(), sizeof(uint32_t) * indexBuf[i].size());
		//	Vindexes += static_cast<uint32_t>(vertexBuf[i].size());
		//	Iindexes += static_cast<uint32_t>(indexBuf[i].size());
		//}

		//printf("\nTotal vertices %d total indices %d total bones %d\n", totalVertices, totalIndices, totalBones);
		return 0;
	}
}