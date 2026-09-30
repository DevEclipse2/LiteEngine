
#pragma once
#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
constexpr uint32_t ENGINE_ABI_VERSION = 1;
typedef uint32_t CallHandle;
constexpr CallHandle INVALID_HANDLE = 0xFFFFFFFF;
typedef uint8_t  plugin_state_call_rule_bits;
constexpr plugin_state_call_rule_bits Enter_call    = 1;
constexpr plugin_state_call_rule_bits Exit_call     = 2;
constexpr plugin_state_call_rule_bits Tick_call     = 4;

//imgui is necessary bloat imo 
enum class EngineState
{

    starting,
    operating,
    critical_exit,
    shutdown
};
enum class EngineSecondaryState
{
    index,
    project,
    debug,
    game,
    quit,
    none
};


class IMemoryAllocator {
public:
    virtual void* Allocate(size_t size, size_t alignment) = 0;
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
        const char** allowedPlugins;
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
    //todo
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
class PreferenceHook
{
public:
    struct fullprefs
    {
        //this dumps the entire thing
        const char** fulldata;
        size_t size;
    };
    virtual void registerCategory(const char* catName) = 0;
    virtual void addKeyValPair(const char* catName, const char* Key, const char* Val) = 0;
    virtual const char* getKeyValPair(const char* catName, const char* Key) = 0;
    virtual fullprefs dumpAllPrefs(const char* catName, const char* Key) = 0;
};
class callRules
{
    //these dictate the call rules for better optimisation
    virtual void setIndexCalls(plugin_state_call_rule_bits bits) = 0;
    virtual void setProjectCalls(plugin_state_call_rule_bits bits) = 0;
    virtual void setDebugCalls(plugin_state_call_rule_bits bits) = 0;
    virtual void setGameCalls(plugin_state_call_rule_bits bits) = 0;
};
//virtual interface
class IEnginePlugin {
public:
    virtual ~IEnginePlugin() = default;
    virtual const char* GetName() const = 0;//only use const for getters and handlers

    // Lifecycle hooks
    //these are called from the engine to the plugins
    virtual void OnBootload() = 0;
    virtual void PreGraphicsCreation        (class IGPUBuilder*     gpu) = 0;
    virtual void OnGraphicsInjection        (class IGPUManager*     gpu) = 0;
    virtual void OnCallAPIDecl              (class CallInterface*   interface) = 0; //done
    virtual void OnDebugAPI                 (class DebugInterface*  interface) = 0;
    virtual void OnPreferenceHook           (class PreferenceHook*  hook) = 0; // done
    virtual void OnWakeMeWhenYouNeedMeHook  (class callRules* rules) = 0;
    virtual void OnPreferencesOutofDate     () = 0;
    virtual void OnHibernation              () = 0;
    virtual void OnIndexOpen                () = 0; //done 
    virtual void OnIndexTick                (float deltaTime) = 0; //done
    virtual void OnIndexExit                (EngineSecondaryState newstate) = 0;
    virtual void OnProjectOpen           () = 0;
    virtual void OnProjectExit              () = 0;
    virtual void OnProjectTick(float deltaTime) = 0;
    virtual void OnDebugOpen() = 0;
    virtual void OnDebugTick(float deltaTime) = 0;
    virtual void OnDebugExit() = 0;
    virtual void OnGameOpen() = 0;
    virtual void OnGameTick(float deltaTime) = 0;
    virtual void OnGameExit() = 0;
};
//these states are defined as follows
/*

Index   : the primary state the engine boots into, and shows a list of prospective projects
Project : a state in which the main focus is the modification of asset files in which the changes are saved and the "game" or "debug" is not actively experiencing time
Debug   : a state in which the "game time" or "debug time" is ticking and able to be interacted with, where the changes are usually not preserved * this can of course be changed to a developer's whim, but its good practise
Game    : usually the build version of the game, and no modifications of the game assets should be encouraged or at least happen without the discresion of the player

*/


//export macros
#if defined(_WIN32)
#define PLUGIN_EXPORT extern "C" __declspec(dllexport) 
#else
#define PLUGIN_EXPORT extern "C"
#endif

PLUGIN_EXPORT IEnginePlugin*    CreatePlugin(IMemoryAllocator* allocator);
PLUGIN_EXPORT const char*       GetName(); //use the INTERNAL NAME
PLUGIN_EXPORT void              DestroyPlugin(IEnginePlugin* plugin);
PLUGIN_EXPORT uint32_t          GetPluginABIVersion();