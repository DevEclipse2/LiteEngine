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
		void checkDependencies();
		bool LoadPlugin(uint16_t index);

		std::string pluginPath;
		bool verifyIntegrity(std::string fpath);

		// Create the global instance
		EngineSystemAllocator g_EngineAllocator;
		//these are state manager functions for the engine
	};
}

