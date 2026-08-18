//dummy class for later

#pragma once
#include <cstdint>

constexpr uint32_t ENGINE_ABI_VERSION = 1;


struct PluginContext {
    void* dummyData;
};

//virtual interface
class IEnginePlugin {
public:
    virtual ~IEnginePlugin() = default;

    virtual bool OnLoad(const PluginContext& ctx) = 0;
    virtual void OnUnload() = 0;
    virtual void OnUpdate(float deltaTime) = 0;
};

//export macros
#if defined(_WIN32)
#define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define PLUGIN_EXPORT extern "C"
#endif