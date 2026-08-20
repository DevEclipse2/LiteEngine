#pragma once
#include "../Assimp/Lt_Importer.h"

namespace lte {

	class InverseKinematics
	{
	public:
		static void SubmitDrawCommands();
		static void Solve(std::set<uint16_t>& set);
		static void RunCCDSolver(const std::vector<uint16_t>& chain, glm::vec3 targetPos, int maxIterations = 10, float threshold = 0.01f);

		static std::vector<uint16_t> FindEndEffectors(const std::vector<Lt_Importer::Node>& hierarchy);
		static std::vector<uint16_t> BuildIKChain(const std::vector<Lt_Importer::Node>& hierarchy, uint16_t endEffectorIndex, int maxChainLength = 3);
	private:
		inline static glm::vec3 ikTargetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		inline static int selectedEndEffectorIndex = -1; // -1 means no bone selected
		inline static int ikChainLength = 3;             // Default to a 3-bone chain
		inline static bool enableIK = false;
		inline static std::vector<uint16_t> leafBones;
	};

}