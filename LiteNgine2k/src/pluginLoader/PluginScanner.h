#pragma once
#include <string>
#include <vector>
#include "ABI.h"
#include <filesystem>
#include <unordered_map>
#pragma once
#include <string>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HMODULE LibraryHandle;
#else
#include <dlfcn.h>
typedef void* LibraryHandle;
#endif

#include "Bridge.h"
#include "ABIPrefs.h"

class EngineSystemAllocator : public IMemoryAllocator {
public:
	void* Allocate(size_t size, size_t alignment) override {
		
		//implement proper allocators
		return std::malloc(size);
	}

	void Free(void* ptr) override {
		std::free(ptr);
	}
};
namespace ltCore {
	class PluginScanner;
}

class PluginFunctionCallRules : public callRules {

public:
	std::string name;
	void setIndexCalls(plugin_state_call_rule_bits bits) override;
	void setProjectCalls(plugin_state_call_rule_bits bits) override;
	void setDebugCalls(plugin_state_call_rule_bits bits) override;
	void setGameCalls(plugin_state_call_rule_bits bits) override;
	static inline ltCore::PluginScanner* host_ptr;
};

namespace ltCore {

	namespace fs = std::filesystem;

	class PlatformLibrary
	{
		//platform agnostic shit
		//this handles the internal states and lifecycle of the plugin

	public:
		static LibraryHandle Load(const fs::path& path) {
#if defined(_WIN32)
			return LoadLibraryA(path.string().c_str());
#else
			return dlopen(path.string().c_str(), RTLD_NOW | RTLD_LOCAL);
#endif
		}

		static void* GetFunction(LibraryHandle handle, const char* funcName) {
#if defined(_WIN32)
			return (void*)GetProcAddress(handle, funcName);
#else
			return dlsym(handle, funcName);
#endif
		}

		static void Unload(LibraryHandle handle) {
#if defined(_WIN32)
			FreeLibrary(handle);
#else
			dlclose(handle);
#endif
		}

	};

	typedef IEnginePlugin* (*CreatePluginFunc)(IMemoryAllocator*);
	typedef const char* (*GetNameFunc)();
	typedef void           (*DestroyPluginFunc)(IEnginePlugin*);
	typedef uint32_t(*GetPluginABIVersionFunc)();
	
	/*
	PLUGIN_EXPORT IEnginePlugin* CreatePlugin(IMemoryAllocator* allocator);
	PLUGIN_EXPORT char* GetName();
	PLUGIN_EXPORT void              DestroyPlugin(IEnginePlugin* plugin);
	PLUGIN_EXPORT uint32_t          GetPluginABIVersion();
	*/
	
	struct PluginDelegate {
		std::string name;
		LibraryHandle osHandle = nullptr;
		IEnginePlugin* instance = nullptr;
		CreatePluginFunc createFunc = nullptr;
		DestroyPluginFunc destroyFunc = nullptr;
		plugin_state_call_rule_bits index_rules		;
		plugin_state_call_rule_bits project_rules	;
		plugin_state_call_rule_bits debug_rules		;
		plugin_state_call_rule_bits game_rules		;
		//this bit is for the rest of them
	};

	


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
			uint32_t ABI_version = 0;
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
		
		std::unordered_map<std::string, uint16_t> lookupTable;
		std::vector<PluginDelegate> activePlugins;
		void Scan();
		void LoadPlugins();
		void AddPlugins();
		

		std::string pluginPath;
		bool verifyIntegrity(std::string fpath);
		void Startup(Bridge* apibridge, preferencesDelegate* prefdel, EngineSecondaryState* current_state, EngineSecondaryState* next_state);
		void Shutdown();

		void IndexEnter();
		void IndexExit();
		void IndexTick(float deltatime);

		void ProjectEnter();
		void ProjectExit();
		void ProjectTick(float deltatime);

		void GameEnter();
		void GameExit();
		void GameTick(float deltatime);

		void DebugEnter();
		void DebugExit();
		void DebugTick(float deltatime);
		// Create the global instance
		Bridge* apibridge;
		preferencesDelegate* prefs;
		EngineSystemAllocator g_EngineAllocator;
		std::vector<PluginFunctionCallRules*> funcCallrules;
		//these are state manager functions for the engine
		static inline std::vector<plugin_state_call_rule_bits> IndexCallBits;
		static inline std::vector<plugin_state_call_rule_bits> ProjectCallBits;
		static inline std::vector<plugin_state_call_rule_bits> DebugCallBits;
		static inline std::vector<plugin_state_call_rule_bits> GameCallBits;
		EngineSecondaryState* current_state_ptr;
		EngineSecondaryState* next_state_ptr;
	private:
		bool LoadPlugin(uint16_t index);
		PluginFunctionCallRules* generateFuncCallRules(std::string& name);
	};
}

