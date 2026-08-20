#pragma once
#include "jraphics/vulkanInstance.h"
namespace ltCore
{
	class Application
	{
public:
		//void Init();
		//void Loop();
		void End();
		//void Cleanup();
		void run();
		vulkanInstance instance{};


	};
}


