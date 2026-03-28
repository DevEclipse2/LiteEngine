#pragma once
#include <GLFW/glfw3.h>
#include "Lt_Window.h"
#include "VulkanDevice.h"
namespace lte 
{
	class main {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
		//NAME = "LiteEngine: Agstrum";
		void run();
	private:
		VulkanDevice vkDevice{};
		Lt_Window ltWindow{ WIDTH, HEIGHT ,"LiteEngine : Agstrum" };
	};
}
