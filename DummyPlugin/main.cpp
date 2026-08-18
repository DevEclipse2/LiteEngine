#include "Plugin_ABI.h"
#include <iostream>
class DummyPlugin : public IEnginePlugin {
public:
    bool OnLoad(const PluginContext& ctx) override {
        std::cout << "[DummyPlugin] Successfully Loaded!\n";
        return true;
    }

    void OnUnload() override {
        std::cout << "[DummyPlugin] Unloading and cleaning up.\n";
    }

    void OnUpdate(float deltaTime) override {
    }
};

// 2. Export the C-ABI Handshake Functions
PLUGIN_EXPORT uint32_t GetPluginAPIVersion() {
    return ENGINE_ABI_VERSION;
}

PLUGIN_EXPORT const char* GetPluginName() {
    return "Dummy Test Plugin";
}

PLUGIN_EXPORT IEnginePlugin* CreatePlugin() {
    // Allocate the plugin instance inside the DLL's heap
    return new DummyPlugin();
}

PLUGIN_EXPORT void DestroyPlugin(IEnginePlugin* plugin) {
    // Safely delete the plugin inside the same DLL heap
    delete plugin;
}