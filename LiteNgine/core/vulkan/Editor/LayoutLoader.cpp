#include "LayoutLoader.h"
#include "../EngineClasses/Lt_Console.h"
#include "../EngineClasses/Lt_Gui.h"
#include "../EngineClasses/Preferences.h"
namespace lte 
{
	void LayoutLoader::LoadLayout(uint8_t id)
	{
		if(id < Layouts.size())
		{
			//use a try catch block
		}
		else 
		{
			Con::LogError("Cannot load layout that is out of index!", MED_SEVERITY, TAG_ENGINE);
		}
	}
	void LayoutLoader::SubmitGUICommands() {
		if (!Enabled) {
			return;
		}
	}

	void LayoutLoader::DrawMenu()
	{
		if (ImGui::BeginMenu("Laouts"))
		{

			//in built layouts
			if (Layouts.size() < Preferences::LayoutLoader::MaxShow)
			{
			}
			else 
			{

			}
			if (ImGui::MenuItem("Undo", "CTRL+Z")) { /* load layout here */ }
			//separator
			//save as button
			//revert button


			ImGui::EndMenu();
		}
	}

}