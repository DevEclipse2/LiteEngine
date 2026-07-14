#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include "../Editor/LtUiWindow.h"
namespace lte 
{
	class LayoutLoader : LtUiWindow
	{
	public:
		struct Lt_UiLayout
		{
			//foreach layout
			//name and layout file name
			std::string name;
			std::string fileName;
			bool isCustom;
			std::vector<std::string> EnabledWindows;
			//vector of base class
		};
		//loads layouts and stuff
		//ltUiWindow is a base class
		std::unordered_map<std::string,LtUiWindow> Windows;
		std::vector<Lt_UiLayout> Layouts = {};


		void Init();
		void ScanDirectories();

		void LoadLayout(size_t id); 
		void CreateNewLayout(const std::string& layoutName);
		void WriteLayoutsToFile(size_t id); // Updates an existing layout file
		void SubmitGUICommands();
		void DrawMenu();
		// button to save, creates popup to prompt, opens / closes the window
		void LoadLayoutFromMemory(const char* ini_data, size_t ini_size);
		std::string SaveLayoutToMemory();
	private:
		std::string LoadDefaultPath;
		std::string LoadCustomPath;
		void ParseEnabledWindows(Lt_UiLayout& layout);

	};

}