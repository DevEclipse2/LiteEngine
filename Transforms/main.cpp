//things to keep in mind
//if you have to, use glfw3dll
//if not, use the inputhandlers given in core litengine









#include "ABI.h"
#include <iostream>
class DummyPlugin : public IEnginePlugin {
public:

};

// 2. Export the C-ABI Handshake Functions
PLUGIN_EXPORT uint32_t GetPluginAPIVersion() {
    return ENGINE_ABI_VERSION;
}

PLUGIN_EXPORT const char* GetPluginName() {
    return "Dummy Test Plugin";
}

PLUGIN_EXPORT void DestroyPlugin(IEnginePlugin* plugin) {
    // Safely delete the plugin inside the same DLL heap
    delete plugin;
}