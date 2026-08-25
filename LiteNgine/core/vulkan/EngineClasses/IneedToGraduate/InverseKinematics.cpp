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
        ImGui::SliderInt("Iterations", &MaxIter, 8, 250);
        ImGui::SliderFloat("tolerance", &tolerance, 0.00001f, 1.0f);

        ImGui::Separator();

        // 4. Execution Toggle
        ImGui::Checkbox("Enable IK Solver", &enableIK);
        ImGui::Checkbox("Enable Animations", &useAnim);

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
        //chain is end to start
        SolveIK_Jacobian(Lt_Importer::SceneNodes,ikChain[0], ikChain[ikChain.size() - 1], ikTargetPosition,MaxIter,tolerance,0.1f);
        //RunCCDSolver(ikChain, ikTargetPosition, 120, 0.1f);
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
    void InverseKinematics::SolveIK_Jacobian(
        std::vector<Lt_Importer::Node>& skeleton,
        int end_effector_idx,
        int root_idx,
        glm::vec3 target_pos,
        int max_iterations,
        float tolerance,
        float lambda // Damping constant to prevent singularity glitches
    ) 
    {
        // 1. Isolate the IK Chain (From Leaf to Root)
        std::vector<int> chain;
        int current = end_effector_idx;
        while (current != -1) {
            chain.push_back(current);
            if (current == root_idx) break; // Stop at the shoulder/hip
            current = skeleton[current].parent;
        }

        float max_step = 0.5f; // Tweak this if it still overshoots

        // 2. The Iterative Solver Loop
        for (int iter = 0; iter < max_iterations; ++iter) {

            std::vector<glm::mat4> global_transforms(skeleton.size(), glm::mat4(1.0f));

            // Iterate backward through the chain vector (which means Root -> Leaf)
            for (int i = chain.size() - 1; i >= 0; --i) {
                int bone_idx = chain[i];
                int parent_idx = skeleton[bone_idx].parent;

                if (parent_idx == -1) {
                    global_transforms[bone_idx] = skeleton[bone_idx].defaultLocalTransform;
                }
                else {
                    global_transforms[bone_idx] = global_transforms[parent_idx] * skeleton[bone_idx].defaultLocalTransform;
                }
            }



            glm::vec3 effector_pos = glm::vec3(global_transforms[end_effector_idx][3]);
            glm::vec3 error = target_pos - effector_pos;

            if (glm::length(error) < tolerance) break;

            // Apply Fix 3: Clamp the error vector so we don't take massive, unstable steps
            if (glm::length(error) > max_step) {
                error = glm::normalize(error) * max_step;
            }

            // C. Build the 3x3 (J * J^T) Matrix
            glm::mat3 JJT(0.0f);
            std::vector<glm::vec3> r_vectors(chain.size());

            for (size_t i = 0; i < chain.size(); ++i) {
                int bone_idx = chain[i];
                glm::vec3 joint_pos = glm::vec3(global_transforms[bone_idx][3]);

                // Vector from current joint to the end-effector
                glm::vec3 r = effector_pos - joint_pos;
                r_vectors[i] = r;

                // Mathematical shortcut: J_i * J_i^T = (r dot r)*I - outerProduct(r, r)
                float r_dot_r = glm::dot(r, r);
                glm::mat3 r_rT = glm::outerProduct(r, r);

                JJT += (r_dot_r * glm::mat3(1.0f)) - r_rT;
            }

            // D. Apply Damped Least Squares to prevent divide-by-zero at singularities
            JJT += glm::mat3(1.0f) * (lambda * lambda);

            // E. Invert the 3x3 matrix and multiply by error to get Spatial Velocity (V)
            glm::vec3 V = glm::inverse(JJT) * error;

            // F. Distribute Rotations back to the Chain (Delta Theta)
            for (size_t i = 0; i < chain.size(); ++i) {
                int bone_idx = chain[i];
                glm::vec3 delta_theta = glm::cross(r_vectors[i], V);
                float angle = glm::length(delta_theta);

                if (angle > 0.00001f) {
                    glm::vec3 axis = delta_theta / angle;
                    glm::quat q_world = glm::angleAxis(angle, axis);
                    glm::mat4 R_world = glm::mat4_cast(q_world);

                    glm::mat4 parent_global = glm::mat4(1.0f);
                    if (skeleton[bone_idx].parent != -1) {
                        parent_global = global_transforms[skeleton[bone_idx].parent];
                    }

                    // NEW FIX 2: Stop the stretching!
                    // 1. Save the original local translation (bone length/offset)
                    glm::vec3 original_translation = glm::vec3(skeleton[bone_idx].defaultLocalTransform[3]);

                    // 2. Calculate the new local matrix
                    glm::mat4 global_old = global_transforms[bone_idx];
                    glm::mat4 global_new = R_world * global_old;
                    glm::mat4 new_local = glm::inverse(parent_global) * global_new;

                    // 3. Force the translation back to exactly what it was
                    new_local[3] = glm::vec4(original_translation, 1.0f);

                    // 4. Save back to the skeleton
                    skeleton[bone_idx].defaultLocalTransform = new_local;
                }
            }
        }
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