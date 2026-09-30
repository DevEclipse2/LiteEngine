#pragma once
#include "ABI.h"
#include <unordered_map>
#include "../forScrap/Bootstrapper.h"
namespace ltCore
{
	class preferencesDelegate;
}

class PrefHook : public PreferenceHook
{
	//these are created on a per dll basis and are tagged 
public:
	void registerCategory(const char* catName) override;
	void addKeyValPair(const char* catName, const char* Key, const char* Val) override;
	const char* getKeyValPair(const char* catName, const char* Key) override;
	fullprefs dumpAllPrefs(const char* catName, const char* Key) override
	{
		fullprefs ret;
		const auto& prefs = lte::Bootstrapper::DumpFullPrefs();
		if (prefs.first == nullptr)
		{
			if (prefs.second == 1)
			{
				ret.fulldata = cachedprefs.first;
				ret.size     = cachedprefs.second;
			}
			else
			{
				//cannot read file
				ret.fulldata = prefs.first;
				ret.size = prefs.second;
				return ret;
			}
		}
		else
		{
			ret.fulldata = prefs.first;
			ret.size = prefs.second;
			cachedprefs = prefs;//loads into memory
		}
		return ret;
	}
	std::string name;//this is used to access functionmap
	inline static ltCore::preferencesDelegate* prefdel;
	static void clearcache()
	{
		if (cachedprefs.first == nullptr) return;

		for (size_t i = 0; i < cachedprefs.second; ++i)
		{
			delete[] cachedprefs.first[i]; // Free each individual C-string
		}

		delete[] cachedprefs.first; // Free the array of pointers itself
	}
private:
	static inline std::pair<const char**, size_t> cachedprefs;
};

namespace ltCore
{
	class preferencesDelegate {
	public:
		std::unordered_map<std::string, PrefHook*> Hooks; // allocated from heap
		PrefHook* createInterface(std::string name);
		void Shutdown();
		void Startup();
	};
}

