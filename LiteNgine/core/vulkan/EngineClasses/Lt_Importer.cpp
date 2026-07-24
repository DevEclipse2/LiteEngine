#include "Lt_Importer.h"
#include "rendering/RenderData.h"
#include <stb_image.h>
#include "../Reworked/Buffers.h"
#include "Lt_Vulkan.h"
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
				data.materialIndex = mesh->mMaterialIndex;
				model.skinnedSubMeshes.push_back(data);
				model.skinnedVertexCount += data.VertexCount;
				model.skinnedIndexCount += data.IndexCount;
			}
			else
			{
				model.transforms.emplace_back(accumulatedTransform);
				Lt_MeshData data;
				ParseMesh(mesh, data);
				data.materialIndex = mesh->mMaterialIndex;
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

	uint8_t Lt_Importer::ParseScene(const aiScene* pScene,const std::string& directory)
	{
		printf("*******************************************************\n");
		printf("Parsing %d meshes\n\n", pScene->mNumMeshes);

		vk::raii::Device& device = Lt_Vulkan::devices[0].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[0].physicalDevice;
		singleTimeCommandInfo cmdInfo{ &device,&Lt_Vulkan::commandPool , &Lt_Vulkan::devices[0].queue };

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
					meshMaterials.emplace_back(submesh.materialIndex);
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
				StaticRenderSet.emplace_back(RenderSet{item.first.startindex,item.first.size,item.second.startindex,item.second.size,meshMaterials[iter],item.first.bufferId,item.second.bufferId,MeshType::Static,overSizedFlags});
				iter++;
			}
			renderSets.insert(renderSets.end(), StaticRenderSet.begin(), StaticRenderSet.end());
		}
		//feeds the skinned vertex buffers
		std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> skinnedVertexAllocators{};
		std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> skinnedIndiceAllocators{};
		std::list<std::pair<AllocationPosition, AllocationPosition>> AllocationPositions; // first is vertex, second is index 
		std::vector<uint32_t> meshMaterials;
		for (const auto& mesh : loadedModels)
		{
			if (mesh.skinnedSubMeshes.size() == 0)continue;
			for (const auto& submesh : mesh.skinnedSubMeshes)
			{
				AllocationPositions.emplace_back(std::pair(AllocationPosition{}, AllocationPosition{}));
				skinnedVertexAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition*>((void*)submesh.skinnedVertexBuffer.data(), submesh.VertexCount, &AllocationPositions.back().first));
				skinnedIndiceAllocators.emplace_back(std::tuple<void*, uint32_t, AllocationPosition*>((void*)submesh.indexBuffer.data(), submesh.IndexCount, &AllocationPositions.back().second));
				meshMaterials.emplace_back(submesh.materialIndex);
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
			skinnedRenderSet.emplace_back(RenderSet{ item.first.startindex,item.first.size,item.second.startindex,item.second.size,meshMaterials[iter],item.first.bufferId,item.second.bufferId,MeshType::Static,overSizedFlags});
			iter++;
		}
		renderSets.insert(renderSets.end(), skinnedRenderSet.begin(), skinnedRenderSet.end());

		//extract all from allocation positions 
		//somehow create rendersets
		return 0;
	}
	uint8_t Lt_Importer::RemoveModels()
	{
		//just this for now
		loadedModels.clear();
		loadedModels.shrink_to_fit();
		return 0;
	}
}