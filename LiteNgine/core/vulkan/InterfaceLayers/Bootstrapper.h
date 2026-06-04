#pragma once
#include <fstream>
#include <iostream>

namespace lte {
	class Bootstrapper
	{
		public:
			

			static std::string preferencesFileName;
			static void OnWake(uint8_t* result);
			static void LoadPrefs();
			static void SavePrefs();
	};
}
