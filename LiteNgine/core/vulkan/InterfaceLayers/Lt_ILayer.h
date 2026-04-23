#pragma once
#include "../Reworked/LtBackend.h"
#include "../Reworked/FileLoader.h"
#include "../Lt_Window.h"
//this is where the main function comes to meet with the usable code
namespace lte {
	
	class LtBackend;
	class Lt_ILayer
	{
		public:
			void Begin();
			void End();
			void Cleanup();
			void Loop();
		private:
			Lt_Window ltWindow{ 800, 600 ,"LiteEngine : Agstrum" };
			LtBackend backend{};
	};
}