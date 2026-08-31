#pragma once
#include "pluginLoader/bridge.h"
#include "jraphics/vulkanInstance.h"
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
		vulkanInstance instance{};


	};
}


