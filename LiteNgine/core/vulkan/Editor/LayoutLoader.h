#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
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
			//vector of base class
		};
		//loads layouts and stuff
		
		std::unordered_map<std::string,LtUiWindow> Windows;
		std::vector<Lt_UiLayout> Layouts = {};
		void CreateNewLayout();
		void LoadLayout(uint8_t id);
		void WriteLayoutsToFile();
		void SubmitGUICommands();
		void DrawMenu();
		// button to save, creates popup to prompt, opens / closes the window
	private:
		std::string LoadDefaultPath;
		std::string LoadCustomPath;
	};

}