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
    struct Node
    {
        ed::NodeId ID;
        std::string Name;
        std::vector<Pin> Inputs;
        std::vector<Pin> Outputs;
        ImColor Color;
        NodeType Type;
        ImVec2 Size;

        std::string State;
        std::string SavedState;

        Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)) :
            ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
        {
        }
    };
	

};

