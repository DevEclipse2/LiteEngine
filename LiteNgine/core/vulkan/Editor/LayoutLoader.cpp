#include "LayoutLoader.h"
#include "../EngineClasses/Lt_Gui.h"
namespace lte 
{
	void LayoutLoader::SubmitGUICommands() {
		if (open) {

			ImGui::Begin("thing", &open);
			ImGui::End();
		}
	}

	void LayoutLoader::DrawMenu()
	{
		if (ImGui::BeginMenu("Laouts"))
		{
			if (ImGui::MenuItem("Undo", "CTRL+Z")) { /* load layout here */ }
			ImGui::EndMenu();
		}
	}

}