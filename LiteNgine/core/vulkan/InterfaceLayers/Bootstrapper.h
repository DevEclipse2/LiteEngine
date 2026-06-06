#pragma once
#include <fstream>
#include <iostream>
#include "../EngineClasses/Lt_Console.h"
namespace lte {
	class Bootstrapper
	{
		public:
			

			static std::string preferencesFileName;
			static void OnWake(uint8_t* result);
			static void LoadPrefs();
			static void SavePrefs();
		private:
			static std::string GenerateFile();
			static void Insert(std::string& str, std::string information);
	};
}
