#include "LayoutLoader.h"
#include "../EngineClasses/Lt_Console.h"
#include "../EngineClasses/Lt_Gui.h"
#include "../EngineClasses/Preferences.h"
namespace lte 
{
	namespace fs = std::filesystem;

	void LayoutLoader::SubmitGUICommands() {
		if (!Enabled) {
			return;
		}
		ImGui::Begin("Load Any Layout here!", NULL, ImGuiWindowFlags_MenuBar);
		/*if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu) 
			{

				ImGui::EndMenu();
			}
			//search, scroll 
			//favourites
			ImGui::EndMenuBar();
		}*/
		ImGui::End();
	}

	void LayoutLoader::LoadLayout(size_t id)
	{
		if (id < Layouts.size())
		{
			//use a try catch block
			const Lt_UiLayout& layout = Layouts[id];

			//gets imgui to load
			ImGui::LoadIniSettingsFromDisk(layout.fileName.c_str());

			// Turn off all windows, then enable only the ones defined in this layout
			for (auto& [windowName, windowObj] : Windows) {
				windowObj.Enabled = false;
			}

			for (const std::string& enabledWin : layout.EnabledWindows) {
				if (Windows.find(enabledWin) != Windows.end()) {
					Windows[enabledWin].Enabled = true;
				}
			}
		}
		else
		{
			Con::LogError("Cannot load layout that is out of index!", MED_SEVERITY, TAG_ENGINE);
		}
		
	}

	void LayoutLoader::CreateNewLayout(const std::string& layoutName)
	{
		// Force custom layouts to go into the custom folder
		std::string path = "custom/" + layoutName + ".ini";

		// Ask ImGui to dump current state to disk
		ImGui::SaveIniSettingsToDisk(path.c_str());

		// Rescan so the new layout instantly appears in our menus and vectors
		ScanDirectories();
	}

	void LayoutLoader::WriteLayoutsToFile(size_t id)
	{
		if (id >= Layouts.size()) return;

		// Overwrite the existing layout file with the current ImGui state
		ImGui::SaveIniSettingsToDisk(Layouts[id].fileName.c_str());
	}

	void LayoutLoader::LoadLayoutFromMemory(const char* ini_data, size_t ini_size)
	{
		ImGui::LoadIniSettingsFromMemory(ini_data, ini_size);
	}

	std::string LayoutLoader::SaveLayoutToMemory()
	{
		size_t out_size = 0;
		const char* saved_data = ImGui::SaveIniSettingsToMemory(&out_size);
		return std::string(saved_data, out_size);
	}

	void LayoutLoader::DrawMenu()
	{
		// Note: Assuming this is called inside an active BeginMenuBar() block
		if (ImGui::BeginMenu("Layouts"))
		{
			if (ImGui::BeginMenu("Main"))
			{
				for (size_t i = 0; i < Layouts.size(); ++i) {
					if (!Layouts[i].isCustom) {
						if (ImGui::MenuItem(Layouts[i].name.c_str())) {
							LoadLayout(i);
						}
					}
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Custom"))
			{
				for (size_t i = 0; i < Layouts.size(); ++i) {
					if (Layouts[i].isCustom) {
						if (ImGui::MenuItem(Layouts[i].name.c_str())) {
							LoadLayout(i);
						}
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			//add popup
			if (ImGui::MenuItem("Save Current as New Custom Layout"))
			{
				CreateNewLayout("My_Custom_Layout");
			}

			ImGui::EndMenu();
		}
	}

	void LayoutLoader::Init() {


		//check exist
		if (!std::filesystem::exists("main"))   std::filesystem::create_directory("main");
		if (!std::filesystem::exists("custom")) std::filesystem::create_directory("custom");

		//scan the files
		ScanDirectories();

		//autoloads the most recent
	}
	void LayoutLoader::ScanDirectories()
	{
		Layouts.clear();

		auto scanFolder = [&](const std::string& folderPath, bool isCustom)
			{
				if (!fs::exists(folderPath)) return;

				for (const auto& entry : fs::directory_iterator(folderPath))
				{
					if (entry.path().extension() == ".ini")
					{
						Lt_UiLayout layout;
						layout.name = entry.path().stem().string(); // Filename without .ini
						layout.fileName = entry.path().string();
						layout.isCustom = isCustom;

						//check which windows are supposed to be running 
						ParseEnabledWindows(layout);
						Layouts.push_back(layout);
					}
				}
			};

		scanFolder("main", false);
		scanFolder("custom", true);
	}

	void LayoutLoader::ParseEnabledWindows(Lt_UiLayout& layout)
	{
		layout.EnabledWindows.clear();

		std::ifstream file(layout.fileName);
		std::string line;
		while (std::getline(file, line))
		{
			// verify if this is an imgui genuine
			if (line.rfind("[Window][", 0) == 0)
			{
				size_t endBracket = line.find(']', 9);
				if (endBracket != std::string::npos)
				{
					std::string winName = line.substr(9, endBracket - 9);
					// Exclude internal nodes
					if (winName.find("DockSpace") == std::string::npos && winName.find("Debug") == std::string::npos)
					{
						layout.EnabledWindows.push_back(winName);
					}
				}
			}
		}
	} 

}