/*
#pragma once
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "../../../dep/backends/imgui.h"
#define UI_TYPE_STRING 0;
#define UI_TYPE_BUTTON 1;
#define UI_TYPE_CHECK  2;
//#define 
namespace lte {
	class UiLayout
	{
		struct Button {
			std::string name;
			bool useScale;
			ImVec2 scale;
			void (*pFunc)();
		};
		struct checkBox {
			std::string name;
			bool* pValue;
		};
		struct Page {
			ImGuiWindowFlags flags;
			bool* open;
			std::vector<uint8_t> type;
			std::vector<Button> buttons; // index of appearance
			std::vector<std::string> strings; // index of appearance
		};
		
		public:
			UiLayout();
			~UiLayout();
			void beginDrawData();
			void insertDrawdata();
			void forceLoadSettings();
			void forceInit();
		private:
			std::string consoleData();
			std::vector<std::string> TabNames = {};
			std::vector<Page> TabPages = {};
			void drawMenu();
			//std::map<std::string, Page> tabs = { { "Viewport", { ImGuiWindowFlags_NoBackground ,nullptr,{0},{},std::vector<std::string>{"testing new UI layout system!"}}} };
	};
}
*/