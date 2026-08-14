#include "Transforms.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <array>
#include "../Lt_Mat.h"
#include "../Lt_Console.h"
namespace lte {

	void Transforms::CalculateTransforms()
	{
		if (DirtyTransforms.size() < TransformComponents.size())
		{
			DirtyTransforms.resize(TransformComponents.size());
		}
		for (int i = 0; i < TransformComponents.size(); i++)
		{
			size_t requiredBytes = (TransformComponents[i].size() + 7) / 8;
			DirtyTransforms[i].resize(requiredBytes, 0); // ensure new memory is zeroed
			bool isBaseNode = (i == 0);
			const std::vector<Transform>& cachedVector = isBaseNode ? TransformComponents[i] : TransformComponents[i - 1];
			uint16_t offset = 0;
			uint8_t byteOffset = 0;
			char isDirtyBits = (requiredBytes > 0) ? DirtyTransforms[i][0] : 0;
			for (int j = 0; j < TransformComponents[i].size(); j++)
			{
				if (byteOffset == 8)
				{
					byteOffset = 0;
					offset++;
					isDirtyBits = DirtyTransforms[i][offset];
				}
				//this is if the current are dirty, or if the parents are dirty
				bool isParentDirty = false;
				if (!isBaseNode)
				{
					for (int k = 0; k < 4; ++k)
					{
						uint16_t targBit = TransformComponents[i][j].activeInfluences[k];
						uint8_t targOffset = targBit % 8;
						char target = DirtyTransforms[i - 1][std::floor(targBit/8)];
						if (target >> targOffset & 1)
						{
							isParentDirty = true;
							break;
						}
					}
				}
				bool isSelfDirty = isDirtyBits >> byteOffset & 1;
				if (isSelfDirty || isParentDirty)
				{
					if (isSelfDirty)
					{
						TransformComponents[i][j].getImmediateTransform();
						//combines the vec3 thingies into mat4
					}
					
					if (TransformComponents[i][j].usageBits & in_use)
					{
						//is valid and available
						if (!isBaseNode)
						{
							std::array<glm::mat4, 4> transforms;
							std::array<float, 4> weights;
							for (int k = 0; k < 4; ++k)
							{
								transforms[k] = cachedVector[TransformComponents[i][j].activeInfluences[k]].AccumulatedTransform;
								if (cachedVector[TransformComponents[i][j].activeInfluences[k]].usageBits & in_use)
								{
									weights[k] = TransformComponents[i][j].influenceWeights[k];
								}
								else
								{
									weights[k] = 0;
									TransformComponents[i][j].influenceWeights[k] = 0;
								}
								
								
							}
							TransformComponents[i][j].AccumulatedTransform = averageTransformsWeighted(transforms, weights) * TransformComponents[i][j].LocalTransform;
						}
						else
						{
							TransformComponents[i][j].AccumulatedTransform = TransformComponents[i][j].LocalTransform;
						}
						
					}
				}

				if (isParentDirty)
				{
					//mark self as dirty
					DirtyTransforms[i][offset] = DirtyTransforms[i][offset] | (1 << byteOffset);
				}
				byteOffset++;
			}
			
		}
		//explodinate all of the dirty bits
		for (int i = 0; i < DirtyTransforms.size(); i++)
		{
			std::fill(DirtyTransforms[i].begin(), DirtyTransforms[i].end(), 0);
		}
	}
	void Transforms::markTransformDirty(uint16_t depth, uint16_t index)
	{
		
		uint16_t packedIndex = index / 8;
		DirtyTransforms[depth][std::floor(packedIndex)] = DirtyTransforms[depth][packedIndex] | (1 << index%8);
	}
	glm::mat4 Transforms::averageTransformsWeighted(
		const std::array<glm::mat4, 4>& matrices,
		const std::array<float, 4>& rawWeights)
	{
		float weightSum = rawWeights[0] + rawWeights[1] + rawWeights[2] + rawWeights[3];
		if (weightSum <= 0.0001f) return glm::mat4(1.0f); // Guard against zero weights

		std::array<float, 4> weights;
		for (int i = 0; i < 4; ++i) {
			weights[i] = rawWeights[i] / weightSum;
		}
		std::array<glm::vec3, 4> translations;
		std::array<glm::quat, 4> rotations;
		std::array<glm::vec3, 4> scales;

		// 2. Decompose all 4 matrices
		for (int i = 0; i < 4; ++i) {
			if (weights[i] > 0.0001f)
			{
				glm::vec3 skew;
				glm::vec4 perspective;
				glm::decompose(matrices[i], scales[i], rotations[i], translations[i], skew, perspective);

				// Ensure quaternions are in the same hemisphere relative to the first one
				if (i > 0 && glm::dot(rotations[0], rotations[i]) < 0.0f) {
					rotations[i] = -rotations[i];
				}
			}
			
		}
		glm::vec3 avgTranslation(0);
		glm::vec3 avgScale(0);
		for (int i = 0; i < 4; ++i)
		{
			if (weights[i] > 0.0001f)
			{
				avgTranslation += translations[i]	* weights[i];
				avgTranslation += scales[i]			* weights[i];
			}
		}
		glm::quat avgRotation = rotations[0];
		float accumulatedWeight = weights[0];

		for (int i = 1; i < 4; ++i) {
			accumulatedWeight += weights[i];
			if (accumulatedWeight > 0.0001f) {
				// The blend factor is the current weight relative to all weight processed so far
				float blendFactor = weights[i] / accumulatedWeight;
				avgRotation = glm::normalize(glm::lerp(avgRotation, rotations[i], blendFactor));
			}
		}

		// 5. Reconstruct the final weighted mat4
		glm::mat4 result = glm::translate(glm::mat4(1.0f), avgTranslation) *
			glm::toMat4(avgRotation) *
			glm::scale(glm::mat4(1.0f), avgScale);

		return result;
	}
	glm::mat4 Transforms::averageTransformsMultiple(const std::vector<glm::mat4>& matrices, const std::vector<float>& rawWeights)
	{
		if (rawWeights.size() != matrices.size())
		{
			Con::LogError("size of vectors not equal when averaging transforms!", HIGH_SEVERITY, TAG_ENGINE);
		}

	}
	glm::mat4 Transforms::GetImmediateGlobalTransforms(uint16_t depth, uint16_t index)
	{
		//it will search its entire heirarchy upwards and calculate every parent
		if (UpdateTransforms.size() < depth) {
			UpdateTransforms.resize(depth);
		}
		UpdateTransforms.resize(depth);
		uint16_t searchDepth = depth;
		uint16_t searchIndex = index;

		for (int i = 0; i < depth; ++i) {
			UpdateTransforms[i].clear();
		}
		//first load all parents of the original node
		for (int i = 0; i < 4; ++i)
		{
			//if parent weight > 0.0001
			if (TransformComponents[searchDepth][searchIndex].influenceWeights[i] > 0.0001f)
			{
				//then it gets added as an influence
				uint16_t parentIndex = TransformComponents[searchDepth][searchIndex].activeInfluences[i];
				if (TransformComponents[searchDepth - 1][parentIndex].usageBits & in_use)
				{
					UpdateTransforms[0].emplace_back(parentIndex);
				}

				//uint8_t byteOffset = parentIndex % 8;
				//if (DirtyTransforms[searchDepth - 1][std::floor(parentIndex / 8)] >> byteOffset & 1)
				//is dirty transform should be added to the thing
			}
		}
		searchDepth--;
		while (searchDepth > 0)
		{
			for (uint16_t childNodeIndex : UpdateTransforms[searchDepth])
			{
				//for each parents, get their parents as well and tack them together
				//then it gets added as an influence

				for (int j = 0; j < 4; ++j)
				{
					if (TransformComponents[searchDepth][childNodeIndex].influenceWeights[j] > 0.0001f)
					{
						uint16_t parentIndex = TransformComponents[searchDepth][childNodeIndex].activeInfluences[j];
						if (TransformComponents[searchDepth - 1][parentIndex].usageBits & in_use)
						{
							// Add to the layer above
							UpdateTransforms[searchDepth - 1].push_back(parentIndex);
						}
					}
				}
			}
			std::sort(UpdateTransforms[0].begin(), UpdateTransforms[0].end());
			UpdateTransforms[0].erase(std::unique(UpdateTransforms[0].begin(), UpdateTransforms[0].end()), UpdateTransforms[0].end());
			searchDepth--;
		}
		
		for (int i = 0; i < UpdateTransforms.size(); i++)
		{
			//iterate throught all transforms and bake them
			//foreach of the base layer parents, iterate and check if they are marked dirty, if they are, calculate transforms and continue
			//if none of their parents and self are dirty bits, dont calculate transforms
			for (auto& currentIndex : UpdateTransforms[i])
			{
				if (i == 0)
				{
					//check if self is dirty
					Transforms::TransformComponents[0][currentIndex].AccumulatedTransform = Transforms::TransformComponents[0][currentIndex].LocalTransform;
				}
				else
				{
					uint8_t byteOffset = currentIndex % 8;
					//check if parents are dirty as well as self
					if (Transforms::DirtyTransforms[i][std::floor(currentIndex / 8)] >> byteOffset & 1)
					{
						// self is dirty

						std::array<glm::mat4, 4> transforms;
						std::array<float, 4> weights;
						for (int k = 0; k < 4; ++k)
						{
							
							transforms[k] = Transforms::TransformComponents[i - 1][Transforms::TransformComponents[i][currentIndex].activeInfluences[k]].AccumulatedTransform;
							if (Transforms::TransformComponents[i - 1][Transforms::TransformComponents[i][currentIndex].activeInfluences[k]].usageBits & in_use)
							{
								weights[k] = Transforms::TransformComponents[i][currentIndex].influenceWeights[k];
							}
							else
							{
								weights[k] = 0;
								Transforms::TransformComponents[i][currentIndex].influenceWeights[k] = 0;
							}
							
						}
						Transforms::TransformComponents[i][currentIndex].AccumulatedTransform = averageTransformsWeighted(transforms, weights) * Transforms::TransformComponents[i][currentIndex].LocalTransform;

					}
					else
					{
						for (int l = 0; l < 4; ++l)
						{
							uint16_t parentIndex = Transforms::TransformComponents[i][currentIndex].activeInfluences[l];
							uint8_t byteOffset = parentIndex % 8;
							if (Transforms::DirtyTransforms[i-1][std::floor(parentIndex / 8)] >> byteOffset & 1)
							{
								std::array<glm::mat4, 4> transforms;
								std::array<float, 4> weights;
								for (int k = 0; k < 4; ++k)
								{

									transforms[k] = Transforms::TransformComponents[i - 1][Transforms::TransformComponents[i][currentIndex].activeInfluences[k]].AccumulatedTransform;
									if (Transforms::TransformComponents[i - 1][Transforms::TransformComponents[i][currentIndex].activeInfluences[k]].usageBits & in_use)
									{
										weights[k] = Transforms::TransformComponents[i][currentIndex].influenceWeights[k];
									}
									else
									{
										weights[k] = 0;
										Transforms::TransformComponents[i][currentIndex].influenceWeights[k] = 0;
									}
								}
								Transforms::TransformComponents[i][currentIndex].AccumulatedTransform = averageTransformsWeighted(transforms, weights) * Transforms::TransformComponents[i][currentIndex].LocalTransform;
								break;
							}
						}
						//check parents
					}
				}
			}
		}
		for (int i = 0; i < UpdateTransforms.size(); i++)
		{
			for (auto& currentIndex : UpdateTransforms[i])
			{
				//at the end mark all of the transforms as clean
				uint8_t byteOffset = currentIndex % 8;
				Transforms::DirtyTransforms[i][std::floor(currentIndex / 8)] = Transforms::DirtyTransforms[i][std::floor(currentIndex / 8)] &= ~(1 << byteOffset);
			}
		}
		//do the final thing 
		std::array<glm::mat4, 4> transforms;
		std::array<float, 4> weights;
		for (int k = 0; k < 4; ++k)
		{
			transforms[k] = Transforms::TransformComponents[depth -1][Transforms::TransformComponents[depth][index].activeInfluences[k]].AccumulatedTransform;
			weights[k] = Transforms::TransformComponents[depth][index].influenceWeights[k];
		}

		Transforms::TransformComponents[depth][index].AccumulatedTransform = averageTransformsWeighted(transforms, weights) * Transforms::TransformComponents[depth][index].LocalTransform;

		return Transforms::TransformComponents[depth][index].AccumulatedTransform;
	}
	
	void Transform::setLocalPosition(glm::vec3& newPos)
	{
		position = newPos; Transforms::markTransformDirty(currentLayer, currentIndex);
	}
	void Transform::setLocalScale(glm::vec3& newScale)
	{
		scale = newScale; Transforms::markTransformDirty(currentLayer, currentIndex);
	}
	void Transform::setLocalRotation(glm::quat& newRotation)
	{
		rotation = newRotation; Transforms::markTransformDirty(currentLayer, currentIndex);
	}
	glm::mat4 Transform::getImmediateTransform()
	{
		glm::mat4 identity = glm::mat4(1.0f);

		glm::mat4 translationMatrix = glm::translate(identity, position);

		glm::quat rotationQuat = rotation;
		glm::mat4 rotationMatrix = glm::toMat4(rotationQuat);
		glm::mat4 scaleMatrix = glm::scale(identity, scale);
		LocalTransform = translationMatrix * rotationMatrix * scaleMatrix;
		return LocalTransform;
	}
	glm::mat4 Transform::getImmGlobalTransform()
	{
		getImmediateTransform();
		if (currentLayer == 0)
		{
			AccumulatedTransform = LocalTransform;
			//also make sure to check the is dirty bit to false
			uint8_t byteOffset = currentIndex % 8;
			Transforms::DirtyTransforms[0][std::floor(currentIndex / 8)] = Transforms::DirtyTransforms[0][std::floor(currentIndex / 8)] &= ~(1 << byteOffset);
			return AccumulatedTransform;
		}
		Transforms::GetImmediateGlobalTransforms(currentLayer, currentIndex);
		//do not use this function too much istg
		//so first go up layers until no parents
		//then for all of the parents check dirty bits and tick them off
		return AccumulatedTransform;
	}
	glm::vec3 Transform::getGlobalPosition()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AccumulatedTransform, scale, rotation, translation, skew,perspective);
		return position;
	}
	glm::quat Transform::getGlobalRotation()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AccumulatedTransform, scale, rotation, translation, skew, perspective);
		return rotation;
	}
	glm::vec3 Transform::getGlobalScale()
	{
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AccumulatedTransform, scale, rotation, translation, skew, perspective);
		return scale;
	}
	void Transform::getGlobalPosVecRot(glm::vec3& position, glm::vec3& scale, glm::quat& rotation)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AccumulatedTransform, scale, rotation, position, skew, perspective);
	}
	glm::vec3 Transform::ImmGlobalScale()
	{
		getImmGlobalTransform();
		return getGlobalScale();
	}
	glm::vec3 Transform::ImmGlobalPosition()
	{
		getImmGlobalTransform();

		return getGlobalPosition();
	}
	glm::quat Transform::ImmGlobalRotation()
	{
		getImmGlobalTransform();
		return getGlobalRotation();
	}
	void Transform::ImmGlobalPosVecRot(glm::vec3& position, glm::vec3& scale, glm::quat& rotation)
	{
		getImmGlobalTransform();
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(AccumulatedTransform, scale, rotation, position, skew, perspective);
	}
}