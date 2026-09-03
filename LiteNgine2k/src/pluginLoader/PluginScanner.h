#pragma once
#include <string>
#include <vector>
namespace ltCore {
	class PluginScanner
	{
		//first scan the plugins directory, find json dependancies
		//find the master
	public:
		enum class dependencyLinkage
		{
			silent,
			warn,
			hard,
		};
		enum class errHandling
		{
			failwarn,
			failskip,
			failerror,
			failthrow,
		};
		struct dependencies
		{
			std::string dependencyName;
			uint32_t versionMin;
			uint32_t versionMax;
			dependencyLinkage linkage;
		};
		struct internalDep
		{
			std::string path;
			dependencyLinkage linkage;
		};
		struct pluginMetaData {
			std::string displayName;
			std::string internalName;
			std::string description;
			int32_t	enginesupportMin;
			int32_t	enginesupportMax;
			errHandling engineVersionIncomptatible;
			errHandling loadingFailed;
			std::string filePath;
			uint16_t internalVersionMajor;
			uint16_t internalVersionMinor;
			uint16_t internalVersionPatch;
			std::vector<dependencies> deps;
			std::vector<internalDep> internalDependencies;
		};
		std::vector<pluginMetaData> metadata;
		void Scan();
		std::string pluginPath;
		bool verifyIntegrity(std::string fpath);
	};
}

