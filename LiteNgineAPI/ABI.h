//dummy class for later

#pragma once
#include <cstdint>

constexpr uint32_t ENGINE_ABI_VERSION = 1;

class IMemoryAllocator {
public:
    virtual void* Allocate(size_t size) = 0;
    virtual void Free(void* ptr) = 0;
};
//virtual interface
class IEnginePlugin {
public:
    virtual const char* GetName() const = 0;

    // Lifecycle hooks
    virtual void OnBootload() = 0;
    virtual void OnGraphicsInjection(class IGPUManager* gpu) = 0;
    virtual void OnHibernation() = 0;
    virtual void OnEditorOpen() = 0;
    virtual void OnEditorTick( float deltaTime) = 0;
    virtual void OnProjectLoading() = 0;
    virtual void OnProjectOpen() = 0;
    virtual void OnProjectTick(float deltaTime) = 0;
    
    virtual void OnProjectClosing() = 0;
    
    virtual void OnGameLoading() = 0;
    virtual void OnGameOpen() = 0;
    virtual void OnGameTick(float deltaTime) = 0;
    virtual void OnGameClosing() = 0;

    virtual void OnQuitEditor() = 0;
};

//export macros
#if defined(_WIN32)
#define PLUGIN_EXPORT extern "C" __declspec(dllexport) 
#else
#define PLUGIN_EXPORT extern "C"
#endif

PLUGIN_EXPORT IEnginePlugin*    CreatePlugin(IMemoryAllocator* allocator);
PLUGIN_EXPORT void              DestroyPlugin(IEnginePlugin* plugin);
