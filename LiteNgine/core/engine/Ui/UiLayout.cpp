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
		TabNames.reserve(1);
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
	void UiLayout::drawMenu() {
		//ImGui::BeginChild();
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Debugging"))
			{
				
				if (ImGui::MenuItem("Start Debug", "")) {}
				if (ImGui::MenuItem("Stop Debug", "")) {}
				if (ImGui::MenuItem("Slow Debug", "")) {}
				if (ImGui::MenuItem("Pause Debug", "")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Workspaces"))
			{
				if (ImGui::MenuItem("Scene", "f1")) {}
				if (ImGui::MenuItem("CodeSpace", "f2")) {}
				if (ImGui::MenuItem("Animation", "f3")) {}
				if (ImGui::MenuItem("Rendering", "f4")) {}
				if (ImGui::MenuItem("Asset Browser", "f5")) {}
				if (ImGui::MenuItem("Cool Hacker Screens (console)", "f5")) {}
				if (ImGui::MenuItem("Game File Creation", "f5")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Documentation", "")) {}
				if (ImGui::MenuItem("Window Creator", "")) {}
				if (ImGui::MenuItem("Summon Jeffery Epstein", "")) {}
				if (ImGui::MenuItem("67", "")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	void UiLayout::beginDrawData()
	{	
		drawMenu();
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
						if (ImGui::Button(page.buttons[indexbuttons].name.c_str()))
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