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
		if (ImGui::BeginMenu("Layouts"))
		{
			// Search Button
			if (ImGui::MenuItem("Search Layouts...")) {
				bOpenSearchPopup = true;
				memset(searchBuffer, 0, sizeof(searchBuffer)); // Clear old search
			}

			ImGui::Separator();

			// Custom Layouts
			if (ImGui::BeginMenu("Load Custom"))
			{
				for (size_t i = 0; i < Layouts.size(); ++i) {
					if (Layouts[i].isCustom) {
						if (ImGui::MenuItem(Layouts[i].name.c_str())) { LoadLayout(i); }
					}
				}
				ImGui::EndMenu();
			}

			// Revert to bakcups
			if (ImGui::BeginMenu("Revert to Main Backup"))
			{
				for (size_t i = 0; i < Layouts.size(); ++i) {
					if (!Layouts[i].isCustom) {
						if (ImGui::MenuItem(Layouts[i].name.c_str())) { LoadLayout(i); }
					}
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();

			// Save As Button
			if (ImGui::MenuItem("Save As New Layout...")) {
				bOpenSaveAsPopup = true;
				memset(saveAsBuffer, 0, sizeof(saveAsBuffer)); // Clear text field
			}

			ImGui::EndMenu();
		}
	}

	void LayoutLoader::Init() {


		//check exist
		if (!std::filesystem::exists("main"))   std::filesystem::create_directory("main");
		if (!std::filesystem::exists("custom")) std::filesystem::create_directory("custom");
		if (!fs::exists("autosave")) fs::create_directory("autosave");
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = "autosave/current_session.ini";
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


	void LayoutLoader::DrawPopups()
	{
		// Handle opening the popups based on the flags set in DrawMenu()
		if (bOpenSaveAsPopup) {
			ImGui::OpenPopup("Save Layout As");
			bOpenSaveAsPopup = false;
		}

		if (bOpenSearchPopup) {
			ImGui::OpenPopup("Search Layouts");
			bOpenSearchPopup = false;
		}

		// --- SAVE AS MODAL ---
		if (ImGui::BeginPopupModal("Save Layout As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter a name for the new layout:");
			ImGui::SetNextItemWidth(250.0f);

			// Use ImGuiInputTextFlags_EnterReturnsTrue to allow hitting 'Enter' to save
			bool hitEnter = ImGui::InputText("##Name", saveAsBuffer, sizeof(saveAsBuffer), ImGuiInputTextFlags_EnterReturnsTrue);

			if (ImGui::Button("Save", ImVec2(120, 0)) || hitEnter) {
				CreateNewLayout(saveAsBuffer);
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// --- SEARCH MODAL ---
		if (ImGui::BeginPopupModal("Search Layouts", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Search:");
			ImGui::SetNextItemWidth(300.0f);
			ImGui::InputText("##SearchBox", searchBuffer, sizeof(searchBuffer));

			ImGui::Separator();

			// Create a scrollable region for the search results
			ImGui::BeginChild("SearchResults", ImVec2(300, 200), true);

			std::string searchStr(searchBuffer);
			// Convert search to lowercase for case-insensitive matching
			std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

			for (size_t i = 0; i < Layouts.size(); ++i)
			{
				std::string layoutNameLower = Layouts[i].name;
				std::transform(layoutNameLower.begin(), layoutNameLower.end(), layoutNameLower.begin(), ::tolower);

				// If search is empty, or the name contains the search string
				if (searchStr.empty() || layoutNameLower.find(searchStr) != std::string::npos)
				{
					// Tag it visually so the user knows if it's Custom or Main
					std::string displayStr = Layouts[i].name + (Layouts[i].isCustom ? " (Custom)" : " (Main)");

					if (ImGui::Selectable(displayStr.c_str())) {
						LoadLayout(i);
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Close", ImVec2(300, 0))) {
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
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