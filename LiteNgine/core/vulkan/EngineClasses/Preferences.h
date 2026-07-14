#pragma once
#include <string>
namespace lte {

	class Preferences 
	{
	public :

		struct LayoutLoader
		{
			inline static int MaxShow = 12;
			inline static std::string CustomLoadPath;
			inline static std::string InBuiltPath;
		};

		struct Graphics
		{
			inline static uint16_t Width = 800;
			inline static uint16_t Height = 600;
		};
		
	};
}