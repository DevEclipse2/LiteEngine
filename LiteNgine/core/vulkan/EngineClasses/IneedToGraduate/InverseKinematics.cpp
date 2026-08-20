#include "InverseKinematics.h"
#include "backends/imgui.h"
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
namespace lte {
    void InverseKinematics::SubmitDrawCommands()
    {
        ImGui::Begin("IK System Debugger");

        // 1. Target Position using DragFloats
        ImGui::Text("Target Position (World Space)");
        // glm::vec3 memory layout is contiguous, so taking the address of x works perfectly
        ImGui::DragFloat3("##IKTarget", &ikTargetPosition.x, 0.05f);

        ImGui::Separator();

        // 2. Leaf Bone Dropdown Menu
        // Get all valid end effectors to populate the dropdown
        if (leafBones.empty())
        {
            leafBones = FindEndEffectors(Lt_Importer::SceneNodes);
        }

        // Determine what to display in the closed dropdown box
        std::string previewName = "Select End Effector...";
        if (selectedEndEffectorIndex != -1 && selectedEndEffectorIndex < Lt_Importer::SceneNodes.size()) {
            previewName = Lt_Importer::SceneNodes[selectedEndEffectorIndex].name;
        }

        ImGui::Text("Kinematic Chain");
        if (ImGui::BeginCombo("End Bone", previewName.c_str())) {
            for (uint16_t leafIndex : leafBones) {
                const bool isSelected = (selectedEndEffectorIndex == leafIndex);

                // Pass the bone's name to the Selectable widget
                if (ImGui::Selectable(Lt_Importer::SceneNodes[leafIndex].name.c_str(), isSelected)) {
                    selectedEndEffectorIndex = leafIndex;
                }

                // Auto-scroll to the selected item when opening the dropdown
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // 3. Chain Length Control
        // Limit the slider from 1 (just the leaf bone) up to a reasonable max like 15
        ImGui::SliderInt("Chain Length", &ikChainLength, 1, 15);

        ImGui::Separator();

        // 4. Execution Toggle
        ImGui::Checkbox("Enable IK Solver", &enableIK);

        // Optional: Visual feedback if no bone is selected but IK is enabled
        if (enableIK && selectedEndEffectorIndex == -1) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Warning: No end effector selected!");
        }

        ImGui::End();
    }
    void InverseKinematics::Solve(std::set<uint16_t>& set)
    {
        if (!enableIK) return;
        if (selectedEndEffectorIndex == -1) return;
        std::vector<uint16_t> ikChain = BuildIKChain(Lt_Importer::SceneNodes, selectedEndEffectorIndex, ikChainLength);
        if (ikChain.empty())
        {
            return;
        }
        RunCCDSolver(ikChain, ikTargetPosition, 120, 0.1f);
        set.insert(ikChain[0]);
    }
    std::vector<uint16_t> InverseKinematics::FindEndEffectors(const std::vector<Lt_Importer::Node>& hierarchy)
    {
        std::vector<uint16_t> endEffectors;

        for (const auto& node : hierarchy) {
            // A leaf node (end bone) has no children
            if (node.children.empty()) {
                endEffectors.push_back(node.selfIndex);
            }
        }

        return endEffectors;
    }
    void InverseKinematics::RunCCDSolver(const std::vector<uint16_t>& chain, glm::vec3 targetPos, int maxIterations, float threshold)
    {
        // A chain must have at least an effector and one parent bone to bend
        if (chain.size() < 2) return;
        uint16_t effectorIndex = chain[0];

        for (int iter = 0; iter < maxIterations; ++iter) {
            // 1. Get current effector position (Column 3 of the accumulated transform matrix)
            glm::vec3 effectorPos = glm::vec3(Lt_Importer::SceneNodes[effectorIndex].AccumulatedTransform[3]);

            // 2. Early Exit: If the effector is close enough to the target, stop iterating
            if (glm::length(targetPos - effectorPos) < threshold) {
                break;
            }

            // 3. Traverse UP the chain from the first parent towards the root
            for (size_t i = 1; i < chain.size(); ++i) {
                uint16_t currentBoneIndex = chain[i];
                Lt_Importer::Node& currentBone = Lt_Importer::SceneNodes[currentBoneIndex];

                // Re-fetch effector pos because previous bone rotations in this loop moved it
                effectorPos = glm::vec3(Lt_Importer::SceneNodes[effectorIndex].AccumulatedTransform[3]);
                glm::vec3 currentPos = glm::vec3(currentBone.AccumulatedTransform[3]);

                // Calculate directional vectors
                glm::vec3 toEffector = glm::normalize(effectorPos - currentPos);
                glm::vec3 toTarget = glm::normalize(targetPos - currentPos);

                // Prevent math domain errors if the vectors are already perfectly aligned
                float cosTheta = glm::dot(toEffector, toTarget);
                if (cosTheta > 0.9999f) continue;

                // 4. Calculate the delta rotation in GLOBAL space
                glm::quat deltaRotGlobal = glm::rotation(toEffector, toTarget);

                // -- THE JITTER FIX: Damping / Slerp --
                // Instead of applying the full rotation, we blend it with the identity quaternion.
                // A weight of 0.5f means it moves halfway to the target per iteration. 
                // Lower = smoother but slower. Higher = faster but more jitter.
                float dampingWeight = 0.5f;
                glm::quat dampedRotGlobal = glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), deltaRotGlobal, dampingWeight);

                glm::mat4 deltaRotMat = glm::mat4_cast(dampedRotGlobal);

                // 5. Apply the damped global rotation
                glm::mat4 globalRotScale = currentBone.AccumulatedTransform;
                globalRotScale[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

                glm::mat4 newGlobalRotScale = deltaRotMat * globalRotScale;

                glm::mat4 newGlobalTransform = newGlobalRotScale;
                newGlobalTransform[3] = glm::vec4(currentPos, 1.0f); // Restore pivot

                // 6. Convert back to LOCAL space (Crucial Math Step)
                // Global_Current = Global_Parent * Local_Current
                // Local_Current = Inverse(Global_Parent) * Global_Current
                glm::mat4 parentGlobal = glm::mat4(1.0f);
                if (currentBone.parent != static_cast<uint16_t>(-1)) {
                    parentGlobal = Lt_Importer::SceneNodes[currentBone.parent].AccumulatedTransform;
                }

                glm::mat4 newLocalTransform = glm::inverse(parentGlobal) * newGlobalTransform;

                // NOTE: In your struct, overwriting defaultLocalTransform destroys your bind pose.
                // You should ideally add a `glm::mat4 currentLocalTransform` to your BoneNode struct.
                // For this snippet to work with your current struct, it assigns it here:
                currentBone.defaultLocalTransform = newLocalTransform;

                // 7. Mini Forward Kinematics Update
                // After rotating this bone, we MUST update the global matrices of all bones 
                // beneath it in the chain before the loop moves to the next parent.
                for (int j = i; j >= 0; --j) {
                    uint16_t updateIndex = chain[j];
                    Lt_Importer::Node& updateBone = Lt_Importer::SceneNodes[updateIndex];

                    glm::mat4 pGlobal = glm::mat4(1.0f);
                    if (updateBone.parent != static_cast<uint16_t>(-1)) {
                        pGlobal = Lt_Importer::SceneNodes[updateBone.parent].AccumulatedTransform;
                    }

                    updateBone.AccumulatedTransform = pGlobal * updateBone.defaultLocalTransform;
                }
            }
        }
    }
    std::vector<uint16_t> InverseKinematics::BuildIKChain(const std::vector<Lt_Importer::Node>& hierarchy, uint16_t endEffectorIndex, int maxChainLength) {
        std::vector<uint16_t> chain;
        uint16_t currentIndex = endEffectorIndex;

        // Trace upwards using the parent indices
        while (currentIndex != static_cast<uint16_t>(-1)) {
            chain.push_back(currentIndex);

            // Stop if we hit a predefined chain limit (e.g., only want to bend arm, not spine)
            if (maxChainLength != -1 && chain.size() >= maxChainLength) {
                break;
            }

            // Stop if we reach the absolute root node (index 0)
            if (currentIndex == 0) {
                break;
            }

            currentIndex = hierarchy[currentIndex].parent;
        }

        // The resulting chain is ordered [EndEffector, Parent, Grandparent, ... Root]
        // This is the exact order required for the CCD algorithm.
        return chain;
    }
}