#pragma once
#include <string>
#include <vector>
namespace ltCore {
	class PluginScanner
	{
		//first scan the plugins directory, find json dependancies
		//find the master
	public:
		struct dependancies
		{
			std::string dependancyName;
			uint32_t versionMin;
			uint32_t versionMax;
		};
		struct pluginMetaData {
			std::string name;
			std::string filePath;
			uint32_t version;
			//dependancies
			//find em!
			std::vector<dependancies> deps;
		};
		std::vector<pluginMetaData> metadata;
		void Scan();
		void ScanDir(std::string filePath);
		std::string pluginPath;
		bool verifyIntegrity(std::string fpath);
	};
}

