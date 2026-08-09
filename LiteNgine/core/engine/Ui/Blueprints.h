#pragma once
#include "imGuiNodes/imgui_node_editor.h"
#include <string>
#include <vector>
//what does blueprints do?
namespace ed = ax::NodeEditor;

class Blueprints
{

    enum class PinKind { Input, Output };
    enum class NodeType {};
    struct Pin {
        ed::PinId ID;
        std::string Name;
        PinKind Kind;
        float Value = 0.0f; // Internal data for calculation
    };

    struct Node {
        ed::NodeId ID;
        std::string Name;
        std::vector<Pin> Inputs;
        std::vector<Pin> Outputs;
        ImVec4 Color;
    };

    struct Link {
        ed::LinkId ID;
        ed::PinId StartPinID;
        ed::PinId EndPinID;
    };

};

