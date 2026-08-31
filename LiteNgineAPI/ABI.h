
#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
constexpr uint32_t ENGINE_ABI_VERSION = 1;
typedef uint32_t CallHandle;
constexpr CallHandle INVALID_HANDLE = 0xFFFFFFFF;
class IMemoryAllocator {
public:
    virtual void* Allocate(size_t size) = 0;
    virtual void Free(void* ptr) = 0;
};
namespace lt {
    struct formlessData
    {
        void* data;
        size_t size;
    };
}
class CallInterface
{
    //this is for calls between plugins to do stuff
    //each dll gets their own interface so the engine knows which calls are from where
public:
    struct func_rules
    {
        //will expand later
        bool isDefaultPublic = true;
        char** allowedPlugins;
        uint16_t count;
    };
    virtual void call(const char* pluginName,const char* func, lt::formlessData* input, lt::formlessData* output) = 0;
    virtual void func_register(void* (lt::formlessData*, lt::formlessData*),const char* name ,func_rules rules) = 0;//this function pointer is held by call interface
    virtual void func_addruleallow(const char* plugin, const char* funcName) = 0;
    virtual CallHandle get_func_handle(const char* targetPlugin, const char* funcName) = 0;
    virtual void call_fast(CallHandle handle, lt::formlessData* input, lt::formlessData* output) = 0;
};
class DebugInterface
{
    //this is for calls between plugins to do stuff
    //each dll gets their own interface so the engine knows which calls are from where
public:
    //basically just console stuff

};
class IGPUBuilder
{
    //assumes that you dont just have 30 ass gpus but all of them are somewhat competent
    //get the physical device, query for features, then request it 
    virtual VkPhysicalDevice getPhysicalDevice(int id) const = 0;
    virtual void requireFeatureStruct(const void* pFeatureStruct, size_t structSize, int gpuID) = 0;
};
class IGPUManager
{
public:
    virtual VkDevice         getDevice(int id) const = 0;
    virtual VkInstance       getInstance() const = 0;
    virtual VkCommandBuffer  getActiveCommandBuffer(int deviceID) const = 0;
    virtual VmaAllocator     getVMAAllocator(int id) const = 0;
    //? stuff here  
    //for dynamic rendering
    virtual VkFormat        GetSwapchainFormat() const = 0;
    virtual VkFormat        GetDepthFormat() const = 0;
    virtual VkQueue         GetQueue() const = 0;
    // Required for Legacy pipeline compilation
    virtual VkRenderPass    GetMainRenderPass() const = 0;
};
//virtual interface
class IEnginePlugin {
public:
    virtual ~IEnginePlugin() = default;
    virtual const char* GetName() const = 0;//only use const for getters and handlers

    // Lifecycle hooks
    //these are called from the engine to the plugins
    virtual void OnBootload() = 0;
    virtual void PreGraphicsCreation(class IGPUBuilder* gpu) = 0;

    virtual void OnGraphicsInjection(class IGPUManager* gpu        ) = 0;
    virtual void OnCallAPIDecl(class CallInterface* interface) = 0;
    virtual void OnDebugAPI(class    DebugInterface* interface) = 0;
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
PLUGIN_EXPORT char*             GetName();
PLUGIN_EXPORT void              DestroyPlugin(IEnginePlugin* plugin);