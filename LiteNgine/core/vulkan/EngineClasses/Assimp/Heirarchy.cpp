#pragma once
#include "Lt_Importer.h"
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