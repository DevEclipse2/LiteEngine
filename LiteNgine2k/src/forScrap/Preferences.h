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

		struct Optimiser
		{

			//for these, they should be limited to 64MB and its multiples; and ideally should be less than 512mb due to it being unlikely it would be used
			//DO NOT UPDATE THESE WHILE ENGINE RUNNING
			inline static uint32_t SkinnedVertexBufferSize = 2097152;// if a skinned vertex is 64 bytes this is 128mb. Perfect!
			inline static uint32_t VertexBufferSize = 3050402; // around 128mb
			inline static uint32_t IndexBufferSize = 33554432; // around 128mb
			//instead of finding the perfect fit , it gets a decent amount of checks before it gives up
			inline static bool UseDefragLimits = true;
			//these can be updated during engine use
			inline static uint8_t maximumSearch = 128;
			inline static uint8_t maximumTrial = 14;
		};
		struct Plugin{
			inline static uint32_t ABIVER; // this is for plugins

		};


		struct Engine {
			inline static bool IsDebug; //production builds will have this off,  leading to things such as no debug logs to save memory
		};
	};
}