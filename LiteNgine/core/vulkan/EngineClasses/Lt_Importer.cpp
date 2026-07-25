#include "Lt_Importer.h"
#include "rendering/RenderData.h"
#include <stb_image.h>
#include "../Reworked/Buffers.h"
#include "Lt_Vulkan.h"
#include "../Reworked/FileLoader.h"
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
		ParseScene(importedScene,"textures/");
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
		data.vertexBuffer.shrink_to_fit();
		data.indexBuffer.shrink_to_fit();
		return 0;
	}

	uint8_t Lt_Importer::ParseSkinnedMesh(aiMesh* mesh, std::vector<Lt_SkinnedMeshData>& outSubMeshes, Model& model)
	{

		Lt_MeshData baseData;		
		ParseMesh(mesh, baseData);
		std::vector<SkinnedProcessorVertex> processorVertices(baseData.vertexBuffer.size());
		std::vector<skinnedVertex> globalVertices;
		globalVertices.reserve(baseData.vertexBuffer.size());

		for (const Vertex& v : baseData.vertexBuffer) {
			globalVertices.push_back(v); // Converts using your constructor
		}

		for (unsigned int i = 0; i < mesh->mNumBones; i++)
		{
			aiBone* bone = mesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			uint8_t globalBoneID;

			if (model.BoneIndexes.find(boneName) == model.BoneIndexes.end()) {
				globalBoneID = static_cast<uint8_t>(model.bones.size());
				model.BoneIndexes[boneName] = globalBoneID;
				Bone newBone;
				newBone.offsetMatrix = ConvertAssimpMatrixToGLM(bone->mOffsetMatrix);
				model.bones.push_back(newBone);
			}
			else {
				globalBoneID = model.BoneIndexes[boneName];
			}

			for (unsigned int w = 0; w < bone->mNumWeights; w++) {
				int vertexId = bone->mWeights[w].mVertexId;
				float weight = bone->mWeights[w].mWeight;
				processorVertices[vertexId].SubmitWeight({ weight, globalBoneID });
			}
		}

		// Resolve the weights into the global vertices
		for (int i = 0; i < globalVertices.size(); i++) {
			processorVertices[i].ResolveWeights(globalVertices[i]);
		}
		processorVertices.clear();

		//mesh partitioner

		const int MAX_BONES_PER_CHUNK = 128;

		// Helper struct for building chunks
		struct MeshChunk {
			Lt_SkinnedMeshData data;
			std::unordered_map<uint8_t, uint8_t> globalToLocalBones;
			std::unordered_map<uint32_t, uint32_t> oldVertexToNewVertex;
		};

		std::vector<MeshChunk> chunks;
		chunks.push_back(MeshChunk()); // Start with one chunk
		chunks[0].data.skinnedVertexBuffer.reserve(baseData.vertexBuffer.size());
		chunks[0].data.indexBuffer.reserve(baseData.indexBuffer.size());
		// Process triangle by triangle to ensure no tearing
		for (size_t i = 0; i < baseData.indexBuffer.size(); i += 3)
		{
			uint32_t idx0 = baseData.indexBuffer[i];
			uint32_t idx1 = baseData.indexBuffer[i + 1];
			uint32_t idx2 = baseData.indexBuffer[i + 2];

			skinnedVertex v0 = globalVertices[idx0];
			skinnedVertex v1 = globalVertices[idx1];
			skinnedVertex v2 = globalVertices[idx2];

			// Collect unique global bones used by this triangle
			std::unordered_set<uint8_t> triangleBones;
			auto extractBones = [&](const skinnedVertex& v) {
				for (int j = 0; j < 4; j++) {
					if (v.BoneWeights[j] > 0.0f) triangleBones.insert(v.BoneIDs[j]);
				}
				};
			extractBones(v0); extractBones(v1); extractBones(v2);

			// Check if the current chunk can hold this triangle's bones
			MeshChunk* currentChunk = &chunks.back();
			int newBonesNeeded = 0;
			for (uint8_t gb : triangleBones) {
				if (currentChunk->globalToLocalBones.find(gb) == currentChunk->globalToLocalBones.end()) {
					newBonesNeeded++;
				}
			}

			// If it exceeds the limit, start a new chunk
			if (currentChunk->globalToLocalBones.size() + newBonesNeeded > MAX_BONES_PER_CHUNK) {
				chunks.push_back(MeshChunk());
				currentChunk = &chunks.back();
			}

			// Add the triangle's bones to the chunk's palette
			for (uint8_t gb : triangleBones) {
				if (currentChunk->globalToLocalBones.find(gb) == currentChunk->globalToLocalBones.end()) {
					uint8_t localId = static_cast<uint8_t>(currentChunk->data.bonePalette.size());
					currentChunk->globalToLocalBones[gb] = localId;
					currentChunk->data.bonePalette.push_back(gb);
				}
			}

			// Helper lambda to add a vertex to the chunk, remapping its BoneIDs to Local IDs
			auto addVertexToChunk = [&](uint32_t oldIdx, skinnedVertex v) -> uint32_t {
				if (currentChunk->oldVertexToNewVertex.find(oldIdx) == currentChunk->oldVertexToNewVertex.end())
				{
					// Remap BoneIDs from Global to Local
					for (int j = 0; j < 4; j++) {
						if (v.BoneWeights[j] > 0.0f) {
							v.BoneIDs[j] = currentChunk->globalToLocalBones[v.BoneIDs[j]];
						}
						else {
							v.BoneIDs[j] = 0;
						}
					}

					uint32_t newIdx = currentChunk->data.skinnedVertexBuffer.size();
					currentChunk->data.skinnedVertexBuffer.push_back(v);
					currentChunk->oldVertexToNewVertex[oldIdx] = newIdx;
					return newIdx;
				}
				return currentChunk->oldVertexToNewVertex[oldIdx];
				};

			// Add the remapped vertices and indices to the chunk
			currentChunk->data.indexBuffer.push_back(addVertexToChunk(idx0, v0));
			currentChunk->data.indexBuffer.push_back(addVertexToChunk(idx1, v1));
			currentChunk->data.indexBuffer.push_back(addVertexToChunk(idx2, v2));
		}

		for (auto& chunk : chunks) {
			chunk.data.VertexCount = chunk.data.skinnedVertexBuffer.size();
			chunk.data.IndexCount = chunk.data.indexBuffer.size();
			chunk.data.materialIndex = mesh->mMaterialIndex;
			outSubMeshes.push_back(std::move(chunk.data));
		}

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
				std::vector<Lt_SkinnedMeshData> data;
				model.skinnedTransforms.emplace_back(accumulatedTransform);
				ParseSkinnedMesh(mesh, data,model);
				model.skinnedSubMeshes.reserve(data.size());
				for (auto& submesh : data)
				{
					submesh.materialIndex = mesh->mMaterialIndex;
					model.skinnedSubMeshes.emplace_back(submesh);
					model.VertexCount += submesh.VertexCount;
					model.IndexCount += submesh.IndexCount;
				}
			}
			else
			{
				model.transforms.emplace_back(accumulatedTransform);
				Lt_MeshData data;
				ParseMesh(mesh, data);
				data.materialIndex = mesh->mMaterialIndex;
				model.subMeshes.push_back(data);
				model.IndexCount	+= data.IndexCount;
				model.VertexCount	+= data.VertexCount;
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ParseNode(node->mChildren[i], scene,model,nodeTransform);
		}
	}

	void Lt_Importer::ParseBoneHeirarchy(const aiNode* node, int parentBoneID, Model& model)
	{
		std::string nodeName = node->mName.C_Str();
		int currentBoneID = parentBoneID;

		// Check if this node is actually a bone we care about
		if (model.BoneIndexes.find(nodeName) != model.BoneIndexes.end())
		{
			currentBoneID = model.BoneIndexes[nodeName];
			Bone& currentBone = model.bones[currentBoneID];

			// 1. Assign the local space transform (Bind Pose)
			currentBone.localTransform = ConvertAssimpMatrixToGLM(node->mTransformation);

			// 2. Link to parent
			currentBone.parentId = parentBoneID;

			// 3. Tell the parent that it has a new child
			if (parentBoneID != -1)
			{
				model.bones[parentBoneID].children.push_back(currentBoneID);
			}
		}

		// Recursively walk down the tree
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ParseBoneHeirarchy(node->mChildren[i], currentBoneID, model);
		}
	}

	uint8_t Lt_Importer::ParseScene(const aiScene* pScene,const std::string& directory)
	{

		//only ever do this once




		printf("*******************************************************\n");
		printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

		vk::raii::Device& device = Lt_Vulkan::devices[0].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[0].physicalDevice;
		singleTimeCommandInfo cmdInfo{ &device,&Lt_Vulkan::commandPool , &Lt_Vulkan::devices[0].queue };

		
		//load the random file only once
		LtImage fallbackImage{};
		FileLoader::createTextureImage("textures/texture.png", fallbackImage,device,PhysicalDevice,cmdInfo);
		fallBackImageIndex = ImageDelegate::requestImageCreation(fallbackImage);



		for (unsigned int i = 0; i < pScene->mNumMaterials; i++)
		{
			aiMaterial* aiMat = pScene->mMaterials[i];
			Lt_Material newMaterial;

			if (aiMat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
			{
				aiString str;
				aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
				std::string texPath = str.C_Str();
				if (loadedTextureMap.find(texPath) != loadedTextureMap.end())
				{
					newMaterial.diffuseTextureIndex = loadedTextureMap[texPath];
				}
				else
				{

					std::string fullPath = directory + "/" + texPath;
			
					int width, height, channels = 0;
					uint32_t mipLevels = 0;

					const aiTexture* embeddedTexture = pScene->GetEmbeddedTexture(texPath.c_str());
					uint8_t* pixels;
					if (embeddedTexture)
					{
						if (embeddedTexture->mHeight == 0)
						{
							pixels = stbi_load_from_memory(
								reinterpret_cast<const stbi_uc*>(embeddedTexture->pcData),
								embeddedTexture->mWidth,
								&width,
								&height,
								&channels,
								STBI_rgb_alpha
							);
						}
						else
						{
							//data is uncompresed
							//rare
							size_t imageByteSize = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;
							width = embeddedTexture->mWidth;
							height = embeddedTexture->mHeight;
							channels = 4;

							pixels = (uint8_t*)malloc(imageByteSize);
							memcpy(pixels, embeddedTexture->pcData, imageByteSize);
						}
					}
					else
					{
						std::string fullPath = directory + "/" + texPath;
						pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
					}
					vk::DeviceSize imageSize = width * height * 4;
					mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
					if (!pixels) {
						throw std::runtime_error("failed to load texture image!");
					}
					vk::raii::Buffer stagingBuffer = nullptr;
					vk::raii::DeviceMemory stagingBufferMemory = nullptr;
					Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, device, PhysicalDevice);

					void* data = stagingBufferMemory.mapMemory(0, imageSize);
					memcpy(data, pixels, imageSize);
					stagingBufferMemory.unmapMemory();

					stbi_image_free(pixels);
					LtImage tmpImg{};

					ImageDelegate::createImage(tmpImg, width, height, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, device, PhysicalDevice);
					ImageDelegate::createImageView(tmpImg, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, 1, device);
					ImageDelegate::createSampler(tmpImg, device);



					ImageDelegate::transitionImageLayout(tmpImg.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, tmpImg.mipLevels, cmdInfo);
					Buffers::copyBufferToImage(stagingBuffer, tmpImg.image, tmpImg.width, tmpImg.height, cmdInfo);
					//transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
					ImageDelegate::generateMipmaps(tmpImg, vk::Format::eR8G8B8A8Srgb, PhysicalDevice, cmdInfo);

					ImageDelegate::createImageView(tmpImg, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, tmpImg.mipLevels, device);
					uint32_t imgIndex = ImageDelegate::requestImageCreation(tmpImg);
					newMaterial.diffuseTextureIndex = imgIndex;
				}
			}
			sceneMaterials.push_back(newMaterial);
		}

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
			ParseBoneHeirarchy(topLevelNode, -1, newObject);
			loadedModels.push_back(newObject);
		}
		return 0;
	}
	uint8_t Lt_Importer::GenerateRenderSets(singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice)
	{
		//this feeds the static vertex buffers first

		//td::vector<std::tuple<void*, uint32_t, AllocationPosition*>> allocPos
		{
			std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> VertexAllocators{};
			std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> IndiceAllocators{};
			std::list<std::pair<AllocationPosition, AllocationPosition>> AllocationPositions; // first is vertex, second is index 
			std::vector<uint32_t> meshMaterials;
			
			for (const auto& mesh : loadedModels)
			{
				if (mesh.subMeshes.size() == 0)continue;
				for (const auto& submesh : mesh.subMeshes)
				{
					AllocationPositions.emplace_back(std::pair (AllocationPosition{}, AllocationPosition{}));
					VertexAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition * >((void*)submesh.vertexBuffer.data(), submesh.VertexCount, &AllocationPositions.back().first));
					IndiceAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition * >((void*)submesh.indexBuffer.data(), submesh.IndexCount, &AllocationPositions.back().second));
					if (mesh.materials.size() == 0)
					{
						meshMaterials.emplace_back(fallBackImageIndex);
					}
					else
					{
						meshMaterials.emplace_back(mesh.materials[submesh.materialIndex].diffuseTextureIndex);
					}
				}
			}
			RenderData::copyBufferContentsBulk(RenderData::BufferType::VertexBuffer, VertexAllocators, info, physicalDevice);
			RenderData::copyBufferContentsBulk(RenderData::BufferType::IndiceBuffer, IndiceAllocators, info, physicalDevice);
			std::vector<RenderSet> StaticRenderSet;
			uint32_t iter = 0;
			for (const auto& item : AllocationPositions)
			{

				//need image index
				uint8_t overSizedFlags = 0;
				if (item.first.IsXL) overSizedFlags |= 1;
				if (item.second.IsXL) overSizedFlags|= 2;
				StaticRenderSet.emplace_back(RenderSet{item.first.startindex,item.first.size,item.second.startindex,item.second.size,meshMaterials[iter],item.first.bufferId,item.second.bufferId,MeshType::Skinned,overSizedFlags});
				iter++;
			}
			renderSets.insert(renderSets.end(), StaticRenderSet.begin(), StaticRenderSet.end());
		}
		//feeds the skinned vertex buffers
		{
			std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> skinnedVertexAllocators{};
			std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> skinnedIndiceAllocators{};
			std::list<std::pair<AllocationPosition, AllocationPosition>> AllocationPositions;
			std::vector<uint32_t> meshMaterials;

			// NEW: We need to store the palettes so we can give them to the RenderSets later
			std::vector<std::vector<uint16_t>> meshBonePalettes;

			for (const auto& mesh : loadedModels)
			{
				if (mesh.skinnedSubMeshes.size() == 0) continue;
				for (const auto& submesh : mesh.skinnedSubMeshes)
				{
					AllocationPositions.emplace_back(std::pair(AllocationPosition{}, AllocationPosition{}));
					skinnedVertexAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition*>((void*)submesh.skinnedVertexBuffer.data(), submesh.VertexCount, &AllocationPositions.back().first));
					skinnedIndiceAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition*>((void*)submesh.indexBuffer.data(), submesh.IndexCount, &AllocationPositions.back().second));

					// Extract Texture and Palette
					if (mesh.materials.size() == 0)
					{
						meshMaterials.emplace_back(fallBackImageIndex);
					}
					else
					{
						meshMaterials.emplace_back(mesh.materials[submesh.materialIndex].diffuseTextureIndex);
					}
					meshBonePalettes.emplace_back(submesh.bonePalette);
				}
			}

			RenderData::copyBufferContentsBulk(RenderData::BufferType::SkinnedVertexBuffer, skinnedVertexAllocators, info, physicalDevice);
			RenderData::copyBufferContentsBulk(RenderData::BufferType::IndiceBuffer, skinnedIndiceAllocators, info, physicalDevice);

			std::vector<RenderSet> skinnedRenderSet;
			uint32_t iter = 0;
			for (const auto& item : AllocationPositions)
			{
				uint8_t overSizedFlags = 0;
				if (item.first.IsXL) overSizedFlags |= 1;
				if (item.second.IsXL) overSizedFlags |= 2;

				// FIX 2: MeshType::Skinned
				// FIX 3: Pass the palette into the RenderSet
				skinnedRenderSet.emplace_back(RenderSet{
					item.first.startindex, item.first.size, item.second.startindex, item.second.size,
					meshMaterials[iter], item.first.bufferId, item.second.bufferId,
					MeshType::Skinned, overSizedFlags, meshBonePalettes[iter]
					});
				iter++;
			}
			renderSets.insert(renderSets.end(), skinnedRenderSet.begin(), skinnedRenderSet.end());
		}
		return 0;
	}
	uint8_t Lt_Importer::RemoveModels()
	{
		//strips the models properly
		uint32_t staticOffset = 0;
		uint32_t skinnedOffset = 0;

		// 2. Find out where the skinned rendersets begin in the global array
		// (It begins exactly after all the static rendersets finish)
		for (const auto& model : loadedModels)
		{
			skinnedOffset += model.subMeshes.size();
		}

		// 3. Extract the models
		for (auto& model : loadedModels)
		{
			StrippedModel newModel;
			newModel.bones = model.bones;
			newModel.BoneIndexes = model.BoneIndexes;
			newModel.name = model.name;
			newModel.transform = model.transform;
			newModel.transforms = model.transforms;

			// Grab Static RenderSets
			if (model.subMeshes.size() > 0)
			{
				newModel.staticRenderset = std::vector<RenderSet>(
					renderSets.begin() + staticOffset,
					renderSets.begin() + staticOffset + model.subMeshes.size()
				);
				staticOffset += model.subMeshes.size();
			}

			// Grab Skinned RenderSets (FIX: assigned to skinnedRenderset!)
			if (model.skinnedSubMeshes.size() > 0)
			{
				newModel.skinnedRenderset = std::vector<RenderSet>(
					renderSets.begin() + skinnedOffset,
					renderSets.begin() + skinnedOffset + model.skinnedSubMeshes.size()
				);
				skinnedOffset += model.skinnedSubMeshes.size();
			}

			// Save the stripped model to your engine's permanent asset list
			strippedModels.push_back(std::move(newModel));
		}

		// Remove all loaded models and free the heavy geometry vectors
		loadedModels.clear();
		loadedModels.shrink_to_fit();

		return 0;
		//since models are loaded linearly,with static rendersets first, you can just load the amount of rendersets = staticmeshes into rendersets.

		//remove all loaded models and frees a bunch of memory
	}
}