#pragma once
#include "Lt_Importer.h"
#include <queue>
namespace lte {
	void Lt_Importer::ParseHeirarchy(const aiNode* CurrentNode,glm::mat4 parentTransform,size_t nodeIndex)
	{

		glm::mat4 nodeTransform = ConvertAssimpMatrixToGLM(CurrentNode->mTransformation);
		glm::mat4 accumulatedTransform = parentTransform * nodeTransform;
		SceneNodes[nodeIndex].defaultLocalTransform = nodeTransform;
		SceneNodes[nodeIndex].name = CurrentNode->mName.C_Str();
		SceneNodes[nodeIndex].children.reserve(CurrentNode->mNumChildren);
		SceneNodes[nodeIndex].referencedModels.reserve(CurrentNode->mNumMeshes);
		SceneNodes[nodeIndex].AccumulatedTransform = accumulatedTransform;
		for (int i = 0; i < CurrentNode->mNumMeshes; i++)
		{
			SceneNodes[nodeIndex].referencedModels.emplace_back(CurrentNode->mMeshes[i]);
		}
		for (unsigned int i = 0; i < CurrentNode->mNumChildren; i++)
		{
			SceneNodes.emplace_back(Node{});
			SceneNodes.back().parent = SceneNodes[nodeIndex].selfIndex;
			SceneNodes.back().selfIndex = SceneNodes.size() - 1;
			SceneNodes[nodeIndex].children.emplace_back(SceneNodes.size() - 1);
			ParseHeirarchy(CurrentNode->mChildren[i],accumulatedTransform,SceneNodes.size() - 1);
		}
	}

	void Lt_Importer::BindBoneParents(Model& rootbone, Node& rootNode)
	{
		//rootbone.parentId = rootNode.selfIndex;
		
		std::unordered_map<std::string, uint16_t> indexes;
		std::queue<uint16_t> nodesToProcess;

		// Initialize the queue with your starting nodes
		for (uint16_t startNodeId : rootNode.children) {
			nodesToProcess.push(startNodeId);
		}
		//dump entire heirarchy
		while (!nodesToProcess.empty())
		{
			uint16_t currentNodeId = nodesToProcess.front();
			nodesToProcess.pop();
			Lt_Importer::Node& node = Lt_Importer::SceneNodes[currentNodeId];
			indexes[node.name] = currentNodeId;
			// Queue up the children for processing later
			for (uint16_t childId : node.children) {
				nodesToProcess.push(childId);
			}
		}

		for (auto& it : rootbone.BoneIndexes)
		{
			if (indexes.find(it.first) != indexes.end())
			{
				rootbone.bones[it.second].parentId = indexes[it.first];
			}
		}

	}


	void Lt_Importer::UpdateTransforms(Node& CurrentNode)
	{
		//multithread this later
		if (CurrentNode.parent != static_cast<uint16_t>(-1))
		{
			CurrentNode.AccumulatedTransform = CurrentNode.defaultLocalTransform * SceneNodes[CurrentNode.parent].AccumulatedTransform;
		}
		else
		{
			CurrentNode.AccumulatedTransform = CurrentNode.defaultLocalTransform;
		}
		for (auto& child : CurrentNode.children)
		{
			UpdateTransforms(SceneNodes[child]);
		}
	}
}