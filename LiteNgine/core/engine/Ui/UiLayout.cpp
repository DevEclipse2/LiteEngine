#include "UiLayout.h"
#include <iostream>
namespace lte {
	UiLayout::UiLayout() {
		/*Page fullViewport{ ImGuiWindowFlags_NoBackground ,nullptr,{0},{},std::vector<std::string>{"testing new UI layout system!"}};
		TabNames.emplace_back("Viewport");
		TabPages.emplace_back(fullViewport);
		Button button{};
		button.name = "test button";
		button.useScale = false;
		button.scale = { 1,1 };
		button.pFunc = []() { std::cout << "button was pressed!\n";};
		Page buttonText{ NULL, nullptr,{1,0},{button},std::vector<std::string>{"Deploying surprise in 3..2..1.."}};
		TabNames.emplace_back("button");
		TabPages.emplace_back(buttonText);*/

	}
	void UiLayout::forceInit() {
		Page fullViewport{ ImGuiWindowFlags_NoBackground ,nullptr,{0},{},std::vector<std::string>{"testing new UI layout system!"} };
		TabNames.emplace_back("Viewport");
		TabPages.emplace_back(fullViewport);
		Button button{};
		button.name = "test button";
		button.useScale = false;
		button.scale = { 1,1 };
		button.pFunc = []() { std::cout << "button was pressed!\n"; };
		Page buttonText{ NULL, nullptr,{1,0},{button},std::vector<std::string>{"Deploying surprise in 3..2..1.."} };
		TabNames.emplace_back("button");
		TabPages.emplace_back(buttonText);
	}
	UiLayout::~UiLayout()
	{
		//frees all pointers
	}

	void UiLayout::beginDrawData()
	{	
		for (int i = 0; i < TabNames.size(); i++) 
		{
			const auto& page = TabPages[i];
			ImGui::Begin(TabNames[i].c_str(),page.open, page.flags);
			uint8_t indexbuttons = 0;
			uint8_t indexstrings = 0;
			for (int i = 0; i < page.type.size(); i++)
			{
				switch (page.type[i]) {
				case 0:
					//std::cout << "string\n";
					ImGui::Text(page.strings[indexstrings].c_str());
					indexstrings++;
					break;
				case 1:
					if (page.buttons[indexbuttons].useScale) {
						if (ImGui::Button(page.buttons[indexbuttons].name.c_str(), page.buttons[indexbuttons].scale))
						{

							page.buttons[indexbuttons].pFunc();
						}
					}
					else {
						if (ImGui::Button(page.buttons[indexbuttons].name.c_str(), page.buttons[indexbuttons].scale))
						{
							page.buttons[indexbuttons].pFunc();
						}
					}
					indexbuttons++;
					break;
				default: std::cerr << "unknown format , terminating...\n";
				}
			}
			ImGui::End();
		}
	}

}