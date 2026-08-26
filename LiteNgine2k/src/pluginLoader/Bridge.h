#pragma once
#include "ABI.h"
#include <set>
#include <unordered_map>

//this is the bridge that is facilitated between the dlls so they go through the center
// 

class DllPort;
namespace ltCore
{
	struct function
	{
		void* (*fn) (lt::formlessData*, lt::formlessData*);
		bool isPublic = true;
		std::set<std::string> allowedPlugins;
	};

	class Bridge
	{
	public:
		DllPort* createInterface(std::string name);
		void Shutdown();
		void Startup();
		std::unordered_map<std::string, std::unordered_map<std::string,uint32_t>> functionMap;
		std::vector<function> functionVec;
		//dllports have fixed positions
		std::unordered_map<std::string, DllPort*> PluginInterfaces; // allocated from heap
	};
}
class DllPort : CallInterface
{
	//these are created on a per dll basis and are tagged 
public:
	void call(const char* pluginName, const char* func, lt::formlessData* input, lt::formlessData* output) override;
	void func_register(void*(lt::formlessData*, lt::formlessData*),const char* name, func_rules rules) override;
	void func_addruleallow(const char* plugin,const char* funcName) override;
	CallHandle get_func_handle(const char* targetPlugin, const char* funcName) override;
	void call_fast(CallHandle handle, lt::formlessData* input, lt::formlessData* output) override;
	//so these guys get their addresses handed over and all calls come through here
	std::string name;
	inline static ltCore::Bridge* bridge;//this is used to access functionmaps
};



