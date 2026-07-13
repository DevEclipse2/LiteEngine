#pragma once
#include <cstdint>
namespace lte 
{
	class LayoutLoader
	{
	public:
		//loads layouts and stuff
		void CreateNewLayout();
		void LoadLayout(uint8_t id);
		void WriteLayoutsToFile();
		void SubmitGUICommands();
		void DrawMenu();

		// button to save, creates popup to prompt, opens / closes the window
	private:
		bool open = true;

	};

}