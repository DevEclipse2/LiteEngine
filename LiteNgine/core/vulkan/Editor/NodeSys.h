#pragma once
#include "../../../dep/imGuiNodes/imgui_node_editor.h"
#include "Viewport.h"
namespace lte {

	

	class NodeSystem
	{
	public:
		void SubmitGUICommands();
		NodeSystem(ax::NodeEditor::EditorContext* m_Context);
		~NodeSystem();
		ax::NodeEditor::EditorContext* m_Context = nullptr;
	private:
		/*void ImGuiEx_BeginColumn();
		void ImGuiEx_NextColumn();
		void ImGuiEx_EndColumn();*/
	};
}

