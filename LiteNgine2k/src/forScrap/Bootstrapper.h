#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include "Lt_Console.h"
#include "Preferences.h"

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
	struct PrefLine 
	{
		std::string key;
		std::string value;
		uint8_t type;
		//1 is normal
		//2 is comment
		//3 is [that one]
	};
	class Bootstrapper
	{
		public:
			
			static std::string preferencesFileName;
			static void OnWake(uint8_t* result);
			static void LoadPrefs(std::string fileName);
			static void SavePrefs(std::string fileName);
			static void DumpPreferences();
			static std::vector<PrefLine> FileLines;
			static std::map<std::string, std::map< std::string ,Preference>> data;
		private:
			static std::string SaveFile();
			static std::string GenerateFile();
			static void Insert(std::string& str, std::string information);
			static std::string LoadPref(std::string name, std::string category);
			template <typename T>
			static T extract(
				const std::string& category,
				const std::string& key,
				T defaultValue);
	};
}
