#include "PluginScanner.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include "../forScrap/Lt_Console.h"
namespace ltCore
{
	using json = nlohmann::json;

	namespace fs = std::filesystem;
	void PluginScanner::Scan()
	{
		std::vector<fs::path> pathes;
		for (const auto& entry : fs::recursive_directory_iterator(pluginPath))
		{
			//recursive scrape
			if (entry.is_regular_file())
			{
				if (entry.path().extension() == ".LiteMeta")
				{
					pathes.push_back(entry.path());
				}
			}

		}
		//then here it loads all 
		for (auto& path : pathes)
		{
			if (!verifyIntegrity(path.string()))
			{
				continue;
			}
			std::ifstream f(path);
			json data = json::parse(f);
			
			// build ver
			// build ver diff action
			// soft dependancies: warn
			// soft dependancies: silent
			// soft dependancies: throw
		}
	}
	bool PluginScanner::verifyIntegrity(std::string fpath)
	{
		lte::SubOp integrityOp{"integrity verification of" + fpath, ""};
		std::ifstream f(fpath);
		json data = json::parse(f);
		//for each key 
		std::array<std::string, 16> AllKeyValPairs =
		{
			"ABI_version",
			"display_name",
			"Internal_name",
			"description",
			"engine_supported_min" ,
			"engine_supported_max" ,
			"fallback_response" ,
			"Internal_Revision_Major" ,
			"Internal_Revision_Minor" ,
			"Internal_Revision_Patch" ,
			"Soft_dependancies_silent",
			"load_fail_response",
			"Soft_dependancies_warn",
			"Hard_dependancies",
			"internal_dependancy_pathes",
			"dep_missing_response",
		};
		bool passed = true;
		for (auto& key : AllKeyValPairs)
		{
			if (!data.contains(key))
			{
				integrityOp.LogError("Missing key: " + key + " in manifest " + fpath +  " ! Check for modifications to metadata or accidental renaming of files to .LiteMeta! LiteMeta files should strictly be used for plugin manifests. Remove or repair this file please!", HIGH_SEVERITY, TAG_ADDON);
				passed = false;
				break;
			}
		}
		if (!passed)
		{
			integrityOp.LogFailure("Failed to verify manifest, This plugin will not be loaded!", MED_SEVERITY, TAG_ADDON);
		}
		else 
		{
			integrityOp.LogSuccess("manifest integrity verified succesfully, proceeding to next step", TAG_ADDON);
		}
		integrityOp.~SubOp();
		return passed;
	}
}