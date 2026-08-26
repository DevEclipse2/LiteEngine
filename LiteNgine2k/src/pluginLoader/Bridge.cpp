#include "Bridge.h"
#include "../forScrap/Lt_Console.h"
void DllPort::call(const char* pluginName, const char* func, lt::formlessData* input, lt::formlessData* output)
{
	//quick rerouting function to do a call to bridge, mainly
    auto pluginIt = bridge->functionMap.find(pluginName);
    if (pluginIt == bridge->functionMap.end())
    {
        lte::Con::LogError("No plugin registered under this name!" + name, HIGH_SEVERITY, TAG_ENGINE);
#ifdef DEBUG
        throw;
#endif // DEBUG
        return;
    }
    auto funcIt = pluginIt->second.find(func);
    if (funcIt == pluginIt->second.end())
    {
        std::string fname = func;
        lte::Con::LogError("Function " + fname + " does not exist!", HIGH_SEVERITY, TAG_ENGINE);
    }
    else
    {
        if (bridge->functionVec[funcIt->second].allowedPlugins.contains(name))
        {
            bridge->functionVec[funcIt->second].fn(input, output);
        }
    }
}

void DllPort::func_register(void* funcptr (lt::formlessData*, lt::formlessData*), const char* funcName, func_rules rules)
{
    //first see if the dll exists or not 
    auto pluginIt = bridge->functionMap.find(name);
    if (pluginIt == bridge->functionMap.end())
    {
        lte::Con::LogError("No plugin registered under this name!" + name, HIGH_SEVERITY, TAG_ENGINE);
#ifdef DEBUG
        throw;
#endif // DEBUG
       return;
    }
    auto funcIt = pluginIt->second.find(funcName);
    if (funcIt == pluginIt->second.end())
    {
        bridge->functionMap[name][funcName] = bridge->functionVec.size();
        ltCore::function func;
        func.isPublic = rules.isDefaultPublic;
        func.fn = funcptr;
        bridge->functionVec.emplace_back(func);
    }
    else
    {
        std::string fname = funcName;
        lte::Con::LogError("Function " + fname + " already registerd!", HIGH_SEVERITY, TAG_ENGINE);

    }
}
void DllPort::func_addruleallow(const char* plugin,const char* funcName) {
    auto pluginIt = bridge->functionMap.find(name);
    if (pluginIt == bridge->functionMap.end())
    {
        lte::Con::LogError("No plugin registered under this name!" + name, HIGH_SEVERITY, TAG_ENGINE);
#ifdef DEBUG
        throw;
#endif // DEBUG
        return;
    }
    auto funcIt = pluginIt->second.find(funcName);
    if (funcIt == pluginIt->second.end())
    {
        std::string fname = funcName;
        lte::Con::LogError("Function " + fname + " does not exist!", HIGH_SEVERITY, TAG_ENGINE);
    }
    else
    {
        bridge->functionVec[funcIt->second].allowedPlugins.emplace(plugin);
    }
}

void DllPort::call_fast(CallHandle handle, lt::formlessData* input, lt::formlessData* output)
{
    bridge->functionVec[handle].fn(input, output);
}
CallHandle DllPort::get_func_handle(const char* targetPlugin, const char* funcName)
{
    //gives a handle for fast calls
    auto targetIt = bridge->functionMap.find(targetPlugin);
    if (targetIt == bridge->functionMap.end()) return INVALID_HANDLE;

    auto funcIt = targetIt->second.find(funcName);
    if (funcIt == targetIt->second.end()) return INVALID_HANDLE;

    uint32_t handle = funcIt->second;

    const ltCore::function& regFunc = bridge->functionVec[handle];
    if (!regFunc.isPublic) {
        bool hasAccess = false;
        for (const auto& allowed : regFunc.allowedPlugins) {
            if (allowed == name) { hasAccess = true; break; }
        }
        if (!hasAccess) return INVALID_HANDLE; // Access denied
    }
    return handle;
}

DllPort* ltCore::Bridge::createInterface(std::string name)
{
    auto it = PluginInterfaces.find(name);
    if (it != PluginInterfaces.end())
    {
        lte::Con::LogError("Attempted to create plugin interface under existing plugin name! may be a duplicaion or security risk!", HIGH_SEVERITY, TAG_ENGINE);
        return nullptr;
    }
    DllPort* port = new DllPort{};
    port->name = name;
    PluginInterfaces[name] = port;
    return port;
}

void ltCore::Bridge::Shutdown()
{
    for (auto& port : PluginInterfaces)
    {
        delete(port.second);
    }
}

void ltCore::Bridge::Startup()
{
    DllPort::bridge = this;
}
