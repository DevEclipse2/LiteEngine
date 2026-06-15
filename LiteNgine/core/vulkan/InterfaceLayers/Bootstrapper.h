#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
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
	struct flattenedData {
		std::string category;
		std::string key;
		Preference preference;
	};
	struct Change
	{
		std::string category;
		std::string KeyNew;
		std::string KeyOld;
		std::string ValNew;
		std::string ValOld;
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
			static std::string SaveFile();
			static std::string GenerateFile();
			static void Insert(std::string& str, std::string information);
	};
}
