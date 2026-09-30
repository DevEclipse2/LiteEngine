#pragma once
#include "pluginLoader/bridge.h"
#include "jraphics/vulkanInstance.h"
#include "pluginLoader/PluginScanner.h"
#include "pluginLoader/ABIPrefs.h"
#include "ABI.h"
namespace ltCore
{
	class Application
	{
public:
		//void Init();
		//void Loop();
		
		

		::EngineState currentState = EngineState::operating;
		::EngineSecondaryState currentSecondaryState = EngineSecondaryState::index;
		::EngineSecondaryState nextSecondaryState = EngineSecondaryState::none;
		void PluginBegin();
		void PluginEnd();
		void MainLoop();
		void End();
		//void Cleanup();
		void run();
		Bridge dllBridge{};
		preferencesDelegate plugin_preferences_delegate{};
		PluginScanner pluginScanner{};
		vulkanInstance instance{};
		void CheckEngine();

	};
}


