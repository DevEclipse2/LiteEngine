#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define in_use 1

namespace lte {
	struct Transform 
	{
	private:
		glm::vec3 position;
		glm::quat rotation;
		glm::vec3 scale;
		glm::mat4 LocalTransform;
		glm::mat4 AccumulatedTransform;
		std::vector<uint16_t> parents;
		std::vector<uint16_t> children;
		glm::u16vec4 activeInfluences;
		glm::vec4 influenceWeights;
		uint16_t currentLayer;
		uint32_t slateID;
		uint16_t currentIndex;
		char usageBits;
			
		friend class Transforms;
	public:
		glm::vec3 getLocalPosition() { return position; }
		void setLocalPosition(glm::vec3& newPos);
		glm::vec3 getLocalScale()	{ return scale;		}
		void setLocalScale(glm::vec3& newScale);
		glm::quat getLocalRotation() { return rotation; }
		void setLocalRotation(glm::quat& newRotation);
		glm::mat4 getLocalTransform() { return LocalTransform; }
		glm::mat4 getGlobalTransform() { return AccumulatedTransform;}
		glm::mat4 getImmediateTransform();//this forces the engine to calculate the local transform
		glm::mat4 getImmGlobalTransform();//these forces the engine to calculate the global
		glm::vec3 getGlobalPosition();//uses accumulated transform to pull
		glm::quat getGlobalRotation();//uses accumulated transform to pull
		glm::vec3 getGlobalScale();//uses accumulated transform to pull
		void getGlobalPosVecRot(glm::vec3& position, glm::vec3& scale, glm::quat& rotation);//uses accumulated transform to pull
		glm::vec3 ImmGlobalScale();		//forces to calculate global transform then pulls
		glm::vec3 ImmGlobalPosition();	//forces to calculate global transform then pulls
		glm::quat ImmGlobalRotation();	//forces to calculate global transform then pulls
	};
	
	class Transforms
	{
		
	public:
		static void CalculateTransforms();//is not for users to call
		//static void RemoveTransform(uint16_t depth, uint16_t index);
		static void markTransformDirty(uint16_t depth, uint16_t index);
		static glm::mat4 averageTransformsWeighted(
			const std::array<glm::mat4, 4>& matrices,
			const std::array<float, 4>& rawWeights);
		//static void RegisterTransform();
	private:
		static inline std::vector<std::vector<Transform>>	TransformComponents;
		static inline std::vector<std::vector<uint16_t>>	freeTransforms;
		static inline std::vector<std::vector<char>>		DirtyTransforms;
	};
}