#pragma once
#include "pluginLoader/bridge.h"
#include "jraphics/vulkanInstance.h"
#include "pluginLoader/PluginScanner.h"
namespace ltCore
{
	class Application
	{
public:
		//void Init();
		//void Loop();
		
		enum class EngineState
		{

		};


		void End();
		//void Cleanup();
		void run();
		Bridge dllBridge{};
		PluginScanner pluginScanner{};
		vulkanInstance instance{};
		void CheckEngine();

	};
}


