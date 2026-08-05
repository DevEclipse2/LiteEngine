#pragma once
#include "Lt_Importer.h"
namespace lte {
	void Lt_Importer::ParseHeirarchy(const aiNode* CurrentNode,glm::mat4 parentTransform,Node LtNode)
	{

		glm::mat4 nodeTransform = ConvertAssimpMatrixToGLM(CurrentNode->mTransformation);
		glm::mat4 accumulatedTransform = parentTransform * nodeTransform;
		LtNode.defaultLocalTransform = nodeTransform;
		LtNode.name = CurrentNode->mName.C_Str();
		LtNode.children.reserve(CurrentNode->mNumChildren);
		LtNode.referencedModels.reserve(CurrentNode->mNumMeshes);
		LtNode.AccumulatedTransform = accumulatedTransform;
		for (int i = 0; i < CurrentNode->mNumMeshes; i++)
		{
			LtNode.referencedModels.emplace_back(CurrentNode->mMeshes[i]);
		}
		for (unsigned int i = 0; i < CurrentNode->mNumChildren; i++)
		{
			SceneNodes.emplace_back(Node{});
			SceneNodes.back().parent = LtNode.selfIndex;
			SceneNodes.back().selfIndex = SceneNodes.size() - 1;
			ParseHeirarchy(CurrentNode->mChildren[i],accumulatedTransform,SceneNodes.back());
		}
	}
	void Lt_Importer::UpdateTransforms(Node CurrentNode)
	{
		//multithread this later
		if (CurrentNode.parent != -1)
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