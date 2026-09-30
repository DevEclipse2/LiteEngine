#include "PluginScanner.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include "../forScrap/Lt_Console.h"
#include "../forScrap/Preferences.h"
namespace ltCore
{
	using json = nlohmann::json;


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


			//manifest checks
			/*
				1 check abi version
				2 check engine support min max
				3 check internal dependancies
				4 get all dependencies
			*/
			pluginMetaData meta{};
			//cast to check abi versions
			//turns abi version into string
			meta.displayName = manifest.display_name;
			meta.internalName = manifest.Internal_name;
			meta.enginesupportMin = std::stoi(manifest.engine_supported_min, nullptr);
			meta.enginesupportMax = std::stoi(manifest.engine_supported_max, nullptr);
			meta.internalVersionPatch = std::stoi(manifest.Internal_Revision_Patch, nullptr);
			meta.internalVersionMinor = std::stoi(manifest.Internal_Revision_Minor, nullptr);
			meta.internalVersionMajor = std::stoi(manifest.Internal_Revision_Major, nullptr);
			meta.ABI_version = std::stoi(manifest.ABI_version, nullptr);

			bool loadFailed = false;


			//this chunk is for load failure handling
			{
				if (manifest.load_fail_response == "failwarn")
				{
					meta.loadingFailed = errHandling::failwarn;

				}
				else if (manifest.load_fail_response == "failthrow")
				{
					meta.loadingFailed = errHandling::failthrow;
				}
				else
				{
					meta.loadingFailed = errHandling::failthrow;
					lte::Con::LogError("no load failure handling recognised!, fallingback to exit on load fail!", MED_SEVERITY, TAG_ADDON);
				}
				if (manifest.fallback_response == "failskip")
				{
					meta.engineVersionIncomptatible = errHandling::failskip;
				}
				else if (manifest.fallback_response == "failerror")
				{
					meta.engineVersionIncomptatible = errHandling::failerror;
				}
				else if (manifest.fallback_response == "failthrow")
				{
					meta.engineVersionIncomptatible = errHandling::failthrow;
				}
				else
				{
					switch (meta.loadingFailed)
					{
					case errHandling::failthrow:
						//quit 
						break;

					case errHandling::failwarn:
						//warn
						loadFailed = true;
						lte::Con::LogWarning("Loading of this module " + meta.displayName + " failed!,skipping over", TAG_ADDON);
						break;
					}
				}
				
			}
			//this chunk is for abi checking
			if (!loadFailed)
			{
				if (meta.ABI_version != lte::Preferences::Plugin::ABI_VER)
				{
					switch (meta.loadingFailed)
					{
					case errHandling::failwarn:
						//warns exits loading 
						//no point loadin if missing dependancies
						loadFailed = true;
						lte::Con::LogWarning("ABI version mismatch in: " + meta.displayName + " , engine using version" + std::to_string(lte::Preferences::Plugin::ABI_VER) + ", but plugin expected"+ std::to_string(meta.ABI_version) + ",skipping over", TAG_ADDON);
						break;
					case errHandling::failthrow:
						//quits the app
						break;
					}
				}
			}
			//olive delights 🎵
			//this chunk is for loading internal dependencies fail erorrs
			if(!loadFailed)
			{
				int iterator = 0;
				//if something is not recognised it triggers a loadfail, response depending on the setting. if the fallback option is not recognised it throws
				for (auto& dependency : manifest.internal_dependency_pathes)
				{
					internalDep dep{};
					dep.path = dependency;
					//dep.linkage = 
					if (manifest.dep_missing_response[iterator] == "warn")
					{
						dep.linkage = dependencyLinkage::warn;
					}
					else if(manifest.dep_missing_response[iterator] == "silent")
					{
						dep.linkage = dependencyLinkage::silent;
					}
					else if (manifest.dep_missing_response[iterator] == "throw")
					{
						dep.linkage = dependencyLinkage::hard;
					}
					else 
					{

						//not recognised
						//use loadfail
						switch (meta.loadingFailed)
						{
						case errHandling::failwarn:
							//warns exits loading 
							//no point loadin if missing dependancies
							loadFailed = true;
							lte::Con::LogWarning("Loading of internal dependancies handling in this module: " + meta.displayName + " has failed! Unrecognized result " + manifest.dep_missing_response[iterator] + ",skipping load", TAG_ADDON);
							break;
						case errHandling::failthrow:
							//quits the app
							break;
						}
					}
					iterator++;
					meta.internalDependencies.emplace_back(dep);
					//it doesnt immediately exit so you can get to catch all of the problems
				}
			}
			//this chunk is for checking if the dependencies exist
			if (!loadFailed) {
				for (auto& dep : meta.internalDependencies)
				{
					fs::path depPath = "";
					depPath.concat<std::string>(lte::Preferences::Plugin::basePath + dep.path);
					if (!fs::exists(depPath))
					{
						switch (meta.loadingFailed)
						{
						case errHandling::failwarn:
							//warns exits loading 
							//no point loadin if missing dependancies
							loadFailed = true;
							
							lte::Con::LogWarning("Loading of internal dependancies in this module: " + meta.displayName + " has failed, no file in path " + depPath.string() + " skipping over", TAG_ADDON);
							break;
						case errHandling::failthrow:
							//quits the app
							break;
						}
					}
					else if (fs::is_directory(depPath))
					{
						switch (meta.loadingFailed)
						{
						case errHandling::failwarn:
							//warns exits loading 
							//no point loadin if missing dependancies
							loadFailed = true;
							lte::Con::LogWarning("Loading of internal dependancies in this module: " + meta.displayName + " has failed, only folders, not directories should be linked! Path: " + depPath.string() + " skipping over", TAG_ADDON);
							break;
						case errHandling::failthrow:
							//quits the app
							break;
						}
					}
				}
			}
			//this chunk is for adding dependencies
			if (!loadFailed) {
				// here check all sizes fit first
				if (manifest.Dependencies.Hard_dependencies.size()								==
					manifest.Dependencies_version_requirement.Max_inclusive.Hard.size()			==
					manifest.Dependencies_version_requirement.Min_inclusive.Hard.size()			
					&&
					manifest.Dependencies.Soft_dependencies_silent.size()						==
					manifest.Dependencies_version_requirement.Max_inclusive.Soft_silent.size()	==
					manifest.Dependencies_version_requirement.Min_inclusive.Soft_silent.size()
					&&
					manifest.Dependencies.Soft_dependencies_warn.size()							==
					manifest.Dependencies_version_requirement.Max_inclusive.Soft_warn.size()	==
					manifest.Dependencies_version_requirement.Min_inclusive.Soft_warn.size())
				{
					for (int i = 0; i < manifest.Dependencies.Hard_dependencies.size(); i++)
					{
						//iterates through all of the array.
						dependencies dependency{};
						dependency.linkage = dependencyLinkage::hard;
						dependency.dependencyName = manifest.Dependencies.Hard_dependencies[i];
						dependency.versionMax = std::stoi(manifest.Dependencies_version_requirement.Max_inclusive.Hard[i], nullptr);
						dependency.versionMin = std::stoi(manifest.Dependencies_version_requirement.Min_inclusive.Hard[i],nullptr);
						meta.deps.push_back(dependency);
					}
					for (int i = 0; i < manifest.Dependencies.Soft_dependencies_silent.size(); i++)
					{
						//iterates through all of the array.
						dependencies dependency{};
						dependency.linkage = dependencyLinkage::silent;
						dependency.dependencyName = manifest.Dependencies.Soft_dependencies_silent[i];
						dependency.versionMax = std::stoi(manifest.Dependencies_version_requirement.Max_inclusive.Soft_silent[i], nullptr);
						dependency.versionMin = std::stoi(manifest.Dependencies_version_requirement.Min_inclusive.Soft_silent[i], nullptr);
						meta.deps.push_back(dependency);
					}
					for (int i = 0; i < manifest.Dependencies.Soft_dependencies_warn.size(); i++)
					{
						//iterates through all of the array.
						dependencies dependency{};
						dependency.linkage = dependencyLinkage::warn;
						dependency.dependencyName = manifest.Dependencies.Soft_dependencies_warn[i];
						dependency.versionMax = std::stoi(manifest.Dependencies_version_requirement.Max_inclusive.Soft_warn[i], nullptr);
						dependency.versionMin = std::stoi(manifest.Dependencies_version_requirement.Min_inclusive.Soft_warn[i], nullptr);
						meta.deps.push_back(dependency);
					}
				}
				else
				{
					switch (meta.loadingFailed)
					{
					case errHandling::failwarn:
						//warns exits loading 
						//no point loadin if missing dependancies
						loadFailed = true;
						lte::Con::LogWarning("dependency array sizes not same! possible modification to this manifest!" + manifest.display_name + " skipping over", TAG_ADDON);
						break;
					case errHandling::failthrow:
						//quits the app
						break;
					}
				}
			}
			if (!loadFailed)
			{
				metadata.push_back(meta);
			}
			else
			{
				//stuff here later
			}
		}		
	}
	void PluginScanner::LoadPlugins()
	{
		
		//first makes sure no duplicates
		std::vector<pluginMetaData> copyMeta;

		for (int i = 0; i < metadata.size(); i++)
		{
			if (lookupTable.contains(metadata[i].internalName))
			{
				//replace if version is superior
				auto& newmeta = metadata[i];
				auto& original = copyMeta[lookupTable[metadata[i].internalName]];
				lte::Con::LogWarning("Multiple versions of the plugin " + newmeta.displayName + " exist! using newest version", TAG_ADDON);
				
				if (original.internalVersionMajor < newmeta.internalVersionMajor)
				{
					copyMeta[lookupTable[newmeta.internalName]] = newmeta;

					//runs the replacement function
				}
				else if (original.internalVersionMajor == newmeta.internalVersionMajor)
				{
					if (original.internalVersionMinor > newmeta.internalVersionMinor)
					{
						copyMeta[lookupTable[newmeta.internalName]] = newmeta;
						//replace
					}
					else if (original.internalVersionMajor == newmeta.internalVersionMajor)
					{
						if (original.internalVersionPatch > newmeta.internalVersionPatch)
						{
							copyMeta[lookupTable[newmeta.internalName]] = newmeta;
							//replace
						}
						else if (original.internalVersionPatch == newmeta.internalVersionPatch)
						{
							lte::Con::LogWarning("same plugin " + newmeta.displayName + " has multiple instances of the same version, if you are a developer, remember to re-run plugin Assembler with new tags", TAG_ADDON);
						}
						else
						{

						}
					}
					else
					{

					}
				}
				else
				{
					//new version is not superior
					//this is an empty block that does nothing until the code review
					//probably a warn somewhere
				}
					
			}
			else 
			{
				lookupTable[metadata[i].internalName] = static_cast<uint16_t>(copyMeta.size());
				copyMeta.push_back(metadata[i]);
			}
		}
		metadata = copyMeta;

		activePlugins.clear();
		activePlugins.reserve(metadata.size());

		for (int i = 0; i < metadata.size(); i++)
		{
			//for all plugins create handler, which is it's single instanced interface, and contains other things 
			//for each plugin,
			//check if the library is there,
			//do lifecycle stuff
			//prep for init and injection
			/*
			* 
			* each one has their own to avoid spoofing
			* 
			* add dll bridge,
			* gpu manager
			* imgui handler
			* debug handler
			* preference handler
			* anything else thats important
			*/
			if (LoadPlugin(i))
			{
				//here it gives the plugin bridge handlers etc
				//gets handle (stored in actual memory and not discarded here)



				//remember failmode
				auto& instance = activePlugins[i].instance;
				instance->OnCallAPIDecl(apibridge->createInterface(activePlugins[i].name));
				instance->OnPreferenceHook(prefs->createInterface(activePlugins[i].name));
				instance->OnWakeMeWhenYouNeedMeHook(generateFuncCallRules(activePlugins[i].name));
				//instance->OnDebugAPI
			}
			else 
			{
				//based on fail do handling

			}
		}

		


		activePlugins.shrink_to_fit();
	}
	bool PluginScanner::LoadPlugin(uint16_t index)
	{
		//plugins are loaded by index via meta files
		//also for this index remember to change the unordered map 
		//hopefully these things aren't called often
		//creates and inserts self
		PluginDelegate plugin;

		// 1. Load the DLL into memory
		fs::path depPath = "";
		depPath = std::filesystem::path(lte::Preferences::Plugin::basePath) / metadata[index].filePath;
		plugin.osHandle = PlatformLibrary::Load(depPath);
		if (!plugin.osHandle)
		{
			//handle response
			lte::Con::LogFailure("Failed to load plugin: " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			return false;
		}

		// 2. Extract the ABI version function FIRST
		auto getVersionFunc = (GetPluginABIVersionFunc)PlatformLibrary::GetFunction(plugin.osHandle, "GetPluginABIVersion");

		if (!getVersionFunc) {
			// Not a valid plugin for this engine
			PlatformLibrary::Unload(plugin.osHandle);
			lte::Con::LogFailure("ABI version get function cannot be found in plugin: " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			return false;
		}

		uint32_t pluginVersion = getVersionFunc();
		if (pluginVersion != lte::Preferences::Plugin::ABI_VER) {
			// Reject the plugin: ABI mismatch
			lte::Con::LogError("Engine ABI Version: " + std::to_string( lte::Preferences::Plugin::ABI_VER) + " vs module version: " + std::to_string(pluginVersion) + " version incompatiblity", MED_SEVERITY, TAG_ADDON);
			lte::Con::LogFailure("ABI version mismatch in plugin: " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			PlatformLibrary::Unload(plugin.osHandle);
			return false;
		}
		if (pluginVersion != metadata[index].ABI_version)
		{
			//reject due to abi declaration mismatch 
			// this will NEVER hit
			lte::Con::LogError("Declared ABI Version: " + std::to_string(metadata[index].ABI_version) + " vs module version: " + std::to_string(pluginVersion) + " version incompatiblity", MED_SEVERITY, TAG_ADDON);
			lte::Con::LogFailure("ABI version declaration mismatch in plugin: " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			PlatformLibrary::Unload(plugin.osHandle);
			return false;
		}

		plugin.createFunc = (CreatePluginFunc)PlatformLibrary::GetFunction(plugin.osHandle, "CreatePlugin");
		plugin.destroyFunc = (DestroyPluginFunc)PlatformLibrary::GetFunction(plugin.osHandle, "DestroyPlugin");
		auto getNameFunc = (GetNameFunc)PlatformLibrary::GetFunction(plugin.osHandle, "GetName");

		if (!plugin.createFunc || !plugin.destroyFunc || !getNameFunc) {

			//unknown erro
			lte::Con::LogFailure("failed to find plugin functions in " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			PlatformLibrary::Unload(plugin.osHandle);
			return false;
		}

		// 5. Initialize
		const char* rawName = getNameFunc();
		plugin.name = rawName ? std::string(rawName) : "Unknown_Plugin";
		//also check name against stuff
		if (plugin.name != metadata[index].internalName)
		{
			lte::Con::LogFailure("internal name mismatch between meta and returned name (" + metadata[index].internalName + ", and " + rawName + ") from " + metadata[index].displayName + "from path " + depPath.string() + ". check engine light flashes", MED_SEVERITY, TAG_ADDON);
			PlatformLibrary::Unload(plugin.osHandle);
			return false;
		}

		plugin.instance = plugin.createFunc(&g_EngineAllocator);

		lookupTable[metadata[index].internalName] = activePlugins.size();

		activePlugins.push_back(plugin);
		return true;

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
	void PluginScanner::Startup(Bridge* dllbridge, preferencesDelegate* prefdel, EngineSecondaryState* current_state, EngineSecondaryState* next_state)
	{
		apibridge = dllbridge;
		prefs = prefdel;
		current_state_ptr = current_state;
		next_state_ptr = next_state;
		PluginFunctionCallRules::host_ptr = this;
	}
	void PluginScanner::Shutdown()
	{
		for (auto& port : funcCallrules)
		{
			delete(port);
		}
		funcCallrules.clear();
		//call plugin destuction


	}
	void PluginScanner::IndexEnter()
	{
		//all plugins are active during the main process
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (IndexCallBits[index] & Enter_call)
			{
				plugin.instance->OnIndexOpen();
			}
			index++;
		}
	}
	void PluginScanner::IndexTick(float deltatime)
	{
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (IndexCallBits[index] & Tick_call)
			{
				plugin.instance->OnIndexTick(deltatime);
			}
			index++;
		}
	}
	void PluginScanner::IndexExit()
	{
		int index = 0;
		if (*next_state_ptr == EngineSecondaryState::none)
		{
			lte::Con::LogError("undefined exit state! indexExit should not be called without a valid exit state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		if (*next_state_ptr == *current_state_ptr)
		{
			lte::Con::LogError("next state is the same as the current state! indexExit should not be called to transition within the same state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		for (const auto& plugin : activePlugins)
		{
			if (IndexCallBits[index] & Exit_call)
			{
				plugin.instance->OnIndexExit(*next_state_ptr);
			}
			index++;
		}
	}
	void PluginScanner::ProjectEnter()
	{
		//all plugins are active during the main process
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (ProjectCallBits[index] & Enter_call)
			{
				plugin.instance->OnProjectOpen();
			}
			index++;
		}
	}
	void PluginScanner::ProjectTick(float deltatime)
	{
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (ProjectCallBits[index] & Tick_call)
			{
				plugin.instance->OnProjectTick(deltatime);
			}
			index++;
		}
	}
	void PluginScanner::ProjectExit()
	{
		int index = 0;
		if (*next_state_ptr == EngineSecondaryState::none)
		{
			lte::Con::LogError("undefined exit state! projectExit should not be called without a valid exit state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		if (*next_state_ptr == *current_state_ptr)
		{
			lte::Con::LogError("next state is the same as the current state! projectExit should not be called to transition within the same state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		for (const auto& plugin : activePlugins)
		{
			if (ProjectCallBits[index] & Exit_call)
			{
				plugin.instance->OnProjectExit(*next_state_ptr);
			}
			index++;
		}
	}
	void PluginScanner::GameEnter()
	{
		//all plugins are active during the main process
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (GameCallBits[index] & Enter_call)
			{
				plugin.instance->OnGameOpen();
			}
			index++;
		}
	}
	void PluginScanner::GameTick(float deltatime)
	{
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (GameCallBits[index] & Tick_call)
			{
				plugin.instance->OnGameTick(deltatime);
			}
			index++;
		}
	}
	void PluginScanner::GameExit()
	{
		int index = 0;
		if (*next_state_ptr == EngineSecondaryState::none)
		{
			lte::Con::LogError("undefined exit state! gameExit should not be called without a valid exit state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		if (*next_state_ptr == *current_state_ptr)
		{
			lte::Con::LogError("next state is the same as the current state! gameExit should not be called to transition within the same state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		for (const auto& plugin : activePlugins)
		{
			if (GameCallBits[index] & Exit_call)
			{
				plugin.instance->OnGameExit(*next_state_ptr);
			}
			index++;
		}
	}
	void PluginScanner::DebugEnter()
	{
		//all plugins are active during the main process
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (DebugCallBits[index] & Enter_call)
			{
				plugin.instance->OnDebugOpen();
			}
			index++;
		}
	}
	void PluginScanner::DebugTick(float deltatime)
	{
		int index = 0;
		for (const auto& plugin : activePlugins)
		{
			if (DebugCallBits[index] & Tick_call)
			{
				plugin.instance->OnDebugTick(deltatime);
			}
			index++;
		}
	}
	void PluginScanner::DebugExit()
	{
		int index = 0;
		if (*next_state_ptr == EngineSecondaryState::none)
		{
			lte::Con::LogError("undefined exit state! debugExit should not be called without a valid exit state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		if (*next_state_ptr == *current_state_ptr)
		{
			lte::Con::LogError("next state is the same as the current state! debugExit should not be called to transition within the same state!", HIGH_SEVERITY, TAG_ADDON);
			return;
		}
		for (const auto& plugin : activePlugins)
		{
			if (DebugCallBits[index] & Exit_call)
			{
				plugin.instance->OnDebugExit(*next_state_ptr);
			}
			index++;
		}
	}

	PluginFunctionCallRules* PluginScanner::generateFuncCallRules(std::string& name)
	{
		
		PluginFunctionCallRules* newrule = new PluginFunctionCallRules{};
		newrule->name = name;
		funcCallrules	.push_back(newrule);
		IndexCallBits	.push_back(Enter_call | Exit_call | Tick_call);
		ProjectCallBits	.push_back(Enter_call | Exit_call | Tick_call);
		DebugCallBits	.push_back(Enter_call | Exit_call | Tick_call);
		GameCallBits	.push_back(Enter_call | Exit_call | Tick_call);
		return newrule;
	}
	
}

void PluginFunctionCallRules::setIndexCalls(plugin_state_call_rule_bits bits)
{
	//remember to add checks here later // done 
	if (host_ptr->lookupTable.contains(name) && host_ptr->IndexCallBits.size() > host_ptr->lookupTable[name])
	{
		host_ptr->IndexCallBits[host_ptr->lookupTable[name]] = bits;
		lte::Con::LogEvent("plugin " + name + " has changed it's index call bits to " + std::to_string(bits) + ".\n refer to documentation for precise meaning.", TAG_ADDON);
		return;
	}
	lte::Con::LogError("cannot find plugin with valid index call bits under " + name + " !", MED_SEVERITY, TAG_ADDON);
}

void PluginFunctionCallRules::setProjectCalls(plugin_state_call_rule_bits bits)
{
	if (host_ptr->lookupTable.contains(name) && host_ptr->ProjectCallBits.size() > host_ptr->lookupTable[name])
	{
		host_ptr->ProjectCallBits[host_ptr->lookupTable[name]] = bits;
		lte::Con::LogEvent("plugin " + name + " has changed it's project call bits to " + std::to_string(bits) + ".\n refer to documentation for precise meaning.", TAG_ADDON);
		return;
	}
	lte::Con::LogError("cannot find plugin with valid project call bits under " + name + " !", MED_SEVERITY, TAG_ADDON);
}

void PluginFunctionCallRules::setDebugCalls(plugin_state_call_rule_bits bits)
{
	if (host_ptr->lookupTable.contains(name) && host_ptr->DebugCallBits.size() > host_ptr->lookupTable[name])
	{
		host_ptr->DebugCallBits[host_ptr->lookupTable[name]] = bits;
		lte::Con::LogEvent("plugin " + name + " has changed it's debug call bits to " + std::to_string(bits) + ".\n refer to documentation for precise meaning.", TAG_ADDON);
		return;
	}
	lte::Con::LogError("cannot find plugin with valid debug call bits under " + name + " !", MED_SEVERITY, TAG_ADDON);
}

void PluginFunctionCallRules::setGameCalls(plugin_state_call_rule_bits bits)
{
	if (host_ptr->lookupTable.contains(name) && host_ptr->GameCallBits.size() > host_ptr->lookupTable[name])
	{
		host_ptr->GameCallBits[host_ptr->lookupTable[name]] = bits;
		lte::Con::LogEvent("plugin " + name + " has changed it's game call bits to " + std::to_string(bits) + ".\n refer to documentation for precise meaning.", TAG_ADDON);
		return;
	}
	lte::Con::LogError("cannot find plugin with valid game call bits under " + name + " !", MED_SEVERITY, TAG_ADDON);
}
