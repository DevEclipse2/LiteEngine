#include "Lt_Importer.h"
namespace lte {
	glm::vec3 Lt_Importer::InterpolatePosition(float animationTime, const BoneTransformTrack& track) {
		if (track.positions.size() == 1) return track.positions[0].position;

		int p0Index = GetPositionIndex(animationTime, track);
		int p1Index = p0Index + 1;

		float scaleFactor = (animationTime - track.positions[p0Index].timeStamp) /
			(track.positions[p1Index].timeStamp - track.positions[p0Index].timeStamp);

		return glm::mix(track.positions[p0Index].position, track.positions[p1Index].position, scaleFactor);
	}

	glm::quat Lt_Importer::InterpolateRotation(float animationTime, const BoneTransformTrack& track) {
		if (track.rotations.size() == 1) return glm::normalize(track.rotations[0].orientation);

		int p0Index = GetRotationIndex(animationTime, track);
		int p1Index = p0Index + 1;

		float scaleFactor = (animationTime - track.rotations[p0Index].timeStamp) /
			(track.rotations[p1Index].timeStamp - track.rotations[p0Index].timeStamp);

		// SLERP (Spherical Linear Interpolation) is required for quaternions
		glm::quat finalRot = glm::slerp(track.rotations[p0Index].orientation, track.rotations[p1Index].orientation, scaleFactor);
		return glm::normalize(finalRot);
	}

	glm::vec3 Lt_Importer::InterpolateScale(float animationTime, const BoneTransformTrack& track) {
		if (track.scales.size() == 1) return track.scales[0].scale;

		int p0Index = GetScaleIndex(animationTime, track);
		int p1Index = p0Index + 1;

		float scaleFactor = (animationTime - track.scales[p0Index].timeStamp) /
			(track.scales[p1Index].timeStamp - track.scales[p0Index].timeStamp);

		return glm::mix(track.scales[p0Index].scale, track.scales[p1Index].scale, scaleFactor);
	}

	int Lt_Importer::GetPositionIndex(float animationTime, const BoneTransformTrack& track) {
		for (int i = 0; i < track.positions.size() - 1; ++i) {
			if (animationTime < track.positions[i + 1].timeStamp)
				return i;
		}
		return 0;
	}

	int Lt_Importer::GetRotationIndex(float animationTime, const BoneTransformTrack& track) {
		for (int i = 0; i < track.rotations.size() - 1; ++i) {
			if (animationTime < track.rotations[i + 1].timeStamp)
				return i;
		}
		return 0;
	}

	int Lt_Importer::GetScaleIndex(float animationTime, const BoneTransformTrack& track) {
		for (int i = 0; i < track.scales.size() - 1; ++i) {
			if (animationTime < track.scales[i + 1].timeStamp)
				return i;
		}
		return 0;
	}
	void Lt_Importer::UpdateAnimation(float animationTime, const Animation& animation, std::set<uint16_t>& updatedNodes)
	{
		//update all nodes, tag the updated ones , then run the cascading updates
		updatedNodes.clear();

		std::unordered_map<std::string_view, const BoneTransformTrack*> trackMap;
		trackMap.reserve(animation.boneTracks.size());
		for (const auto& track : animation.boneTracks)
		{
			trackMap[track.boneName] = &track;
		}

		//Iterate through all nodes and find pairings
		for (size_t i = 0; i < SceneNodes.size(); ++i)
		{
			auto& node = SceneNodes[i];

			// Use string_view lookup to avoid string allocation overhead
			auto it = trackMap.find(node.name);

			if (it != trackMap.end())
			{
				const BoneTransformTrack* track = it->second;

				// Interpolate keyframes
				glm::vec3 pos = InterpolatePosition(animationTime, *track);
				glm::quat rot = InterpolateRotation(animationTime, *track);
				glm::vec3 scl = InterpolateScale(animationTime, *track);

				// Compute local matrix: Scale -> Rotate -> Translate
				glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
				glm::mat4 rotation = glm::mat4_cast(rot); // Use explicit cast for safety
				glm::mat4 scale = glm::scale(glm::mat4(1.0f), scl);

				// Assign the newly animated local matrix to the node
				node.defaultLocalTransform = translation * rotation * scale;

				// Tag this node ID as updated for your cascading hierarchy pass
				updatedNodes.emplace(static_cast<uint16_t>(i));
			}
			else
			{
				node.defaultLocalTransform = node.defaultLocalTransform;
			}
		}
	}

	void Lt_Importer::UpdateBoneMatrices(StrippedModel& model, LtSkinnedMeshInfo& meshInfo)
	{
		//for each bone use node graph to find transforms
		uint32_t index = 0;
		for (auto& it : model.bones)
		{
			if (it.parentId == static_cast<uint16_t>(-1))
			{
				meshInfo.finalBoneMatrices[index] = it.offsetMatrix;
			}
			else 
			{
				meshInfo.finalBoneMatrices[index] = Lt_Importer::SceneNodes[it.parentId].AccumulatedTransform * it.offsetMatrix;
			}
			index++;
		}
	}

	void Lt_Importer::UpdateHierarchy(const Node& node, const glm::mat4& parentTransform, float animationTime, StrippedModel& model, LtSkinnedMeshInfo& meshInfo, const Animation& animation)
	{
		glm::mat4 nodeTransform = node.defaultLocalTransform;
		std::cout << "Trying to animate node: " << node.name << '\n';
		//If animated, override with interpolated keyframes
		const BoneTransformTrack* track = FindBoneTrack(animation, node.name);
		if (track)
		{
			glm::vec3 pos = InterpolatePosition(animationTime, *track);
			glm::quat rot = InterpolateRotation(animationTime, *track);
			glm::vec3 scl = InterpolateScale(animationTime, *track);

			glm::mat4 translation = glm::translate(glm::mat4(1.0f), pos);
			glm::mat4 rotation = glm::mat4(rot);
			glm::mat4 scale = glm::scale(glm::mat4(1.0f), scl);

			nodeTransform = translation * rotation * scale;
		}

		//Accumulate global transform
		glm::mat4 globalTransform = parentTransform * nodeTransform;

		//Calculate Final Bone Matrix for the Shader
		if (model.BoneIndexes.find(node.name) != model.BoneIndexes.end())
		{
			uint32_t boneIndex = model.BoneIndexes[node.name];

			meshInfo.finalBoneMatrices[boneIndex] = globalTransform * model.bones[boneIndex].offsetMatrix;
		}

		//Pass to children
		for (const auto& childNode : node.children)
		{
			UpdateHierarchy(SceneNodes[childNode], globalTransform, animationTime, model, meshInfo, animation);
		}
	}

	const Lt_Importer::BoneTransformTrack* Lt_Importer::FindBoneTrack(const Animation& animation, const std::string& nodeName)
	{
		for (const auto& track : animation.boneTracks) {
			if (track.boneName == nodeName) {
				return &track;
			}
		}
		return nullptr;
	}
}