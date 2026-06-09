#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include "../EngineClasses/Lt_Console.h"
#define DTYPE_STRING	0;
#define DTYPE_INT		1;
#define DTYPE_FLOAT		2;
namespace lte {



	struct Preference
	{
		std::string value;
		uint16_t lineNum;
	};

	class Bootstrapper
	{
		public:
			

			static std::string preferencesFileName;
			static void OnWake(uint8_t* result);
			static void LoadPrefs(std::string fileName);
			static void SavePrefs(std::string fileName);

			static std::map<std::string, std::map< std::string ,Preference>> data;
			static std::map<uint16_t, std::string> comments;
		private:
			static std::string GenerateFile();
			static void Insert(std::string& str, std::string information);
	};
}
