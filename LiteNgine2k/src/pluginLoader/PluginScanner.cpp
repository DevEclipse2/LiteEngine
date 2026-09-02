#include "PluginScanner.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
#include "../forScrap/Lt_Console.h"
#include "../forScrap/Preferences.h"
namespace ltCore
{
	using json = nlohmann::json;

	namespace fs = std::filesystem;

	struct dependencies {
		std::vector<std::string> Soft_dependencies_silent;
		std::vector<std::string> Soft_dependencies_warn;
		std::vector<std::string> Hard_dependencies;
	};

	struct Version {
		std::vector<std::string> Soft_silent;
		std::vector<std::string> Soft_warn;
		std::vector<std::string> Hard;
	};

	struct VersionRequirement {
		Version Min_inclusive;
		Version Max_inclusive;
	};

	struct PluginManifest {
		std::string ABI_version;
		std::string display_name;
		std::string Internal_name;
		std::string description;
		std::string engine_supported_min;
		std::string engine_supported_max;
		std::string fallback_response;
		std::string load_fail_response;
		std::string Internal_Revision_Major;
		std::string Internal_Revision_Minor;
		std::string Internal_Revision_Patch;
		dependencies Dependencies;
		VersionRequirement Dependencies_version_requirement;
		std::vector<std::string> internal_dependency_pathes;
		std::vector<std::string> dep_missing_response;
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(dependencies,
		Soft_dependencies_silent, Soft_dependencies_warn, Hard_dependencies)

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Version,
		Soft_silent, Soft_warn, Hard)

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(VersionRequirement,
		Min_inclusive, Max_inclusive)

	// This macro auto-generates the from_json and to_json mapping
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PluginManifest,
		ABI_version, display_name, Internal_name, description,
		engine_supported_min, engine_supported_max, fallback_response,
		load_fail_response, Internal_Revision_Major, Internal_Revision_Minor,
		Internal_Revision_Patch,
		Dependencies,
		Dependencies_version_requirement,
		internal_dependency_pathes, dep_missing_response
	)



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
				lte::Con::LogFailure("failed to verify manifest integrity,skipping load :" + path.string(), MED_SEVERITY, TAG_ADDON);
				continue;

			}
			lte::Con::LogSuccess("successfully verified manifest integrity of : " + path.string(), TAG_ADDON);

			std::ifstream f(path);
			json data = json::parse(f);
			PluginManifest manifest = data.get<PluginManifest>();

			bool ThrowOnLoadFail = false;

			//manifest checks
			/*
				1 check abi version
				2 check engine support min max
				3 check internal dependancies
			*/

			if

			//this chunk is for load failure handling
			{
				if (manifest.load_fail_response == "failwarn")
				{

				}
				else if (manifest.load_fail_response == "failthrow")
				{
					ThrowOnLoadFail = true;
				}
				else
				{
					lte::Con::LogError("no load failure handling recognised!, fallingback to exit on load fail!", MED_SEVERITY, TAG_ADDON);
					ThrowOnLoadFail = true;
				}
			}
			


			//if something is not recognised it triggers a loadfail, response depending on the setting. if the fallback option is not recognised it throws
			int iterator = 0;
			for (auto& internalDep : manifest.internal_dependency_pathes)
			{
				if (!fs::exists(internalDep))
				{
					
					switch ()
					{

					default:
					}
				}
				iterator++;
			}


		}
	}
	bool PluginScanner::verifyIntegrity(std::string fpath)
	{
		lte::SubOp integrityOp{"integrity verification of" + fpath, ""};
		std::ifstream f(fpath);
		json data = json::parse(f);
		//for each key 
		std::array<std::string, 15> AllKeyValPairs =
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
			"load_fail_response"	  ,
			"dependencies",
			"dependencies_version_requirement",
			"internal_dependency_pathes",
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