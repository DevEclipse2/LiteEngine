#pragma once
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/FileLoader.h"
#include "../EngineClasses/Lt_Vulkan.h"
#include "../EngineClasses/Lt_Console.h"
#include "../../../dep/imGuiNodes/imgui_node_editor.h"
namespace lte {
	class Viewport
	{
	public:
		void SubmitGUICommands();
		void Init();
		void Terminate();
		Viewport();
		~Viewport();
		ax::NodeEditor::EditorContext* m_Context = nullptr;

	private:
		float f = 1.0f;

		GUI_Image image{};
		uint32_t viewportImageIndex = 0;
		void ImGuiEx_BeginColumn();
		void ImGuiEx_NextColumn();
		void ImGuiEx_EndColumn();
	};	

}