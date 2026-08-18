// LiteNgine2k.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "include/Plugin_ABI.h"
#include <filesystem>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>

// Function pointer signatures matching the plugin exports
typedef uint32_t(*GetPluginAPIVersionFn)();
typedef const char* (*GetPluginNameFn)();
typedef IEnginePlugin* (*CreatePluginFn)();
typedef void (*DestroyPluginFn)(IEnginePlugin*);

int main() {
    std::cout << "[Host] LiteNgine Core Host booting...\n";

    // 1. Locate the compiled DLL
    const char* dllPath = "DummyPlugin.dll";
    if (!std::filesystem::exists(dllPath)) {
        std::cerr << "[Host] Error: Could not find '" << dllPath
            << "' in the working directory.\n";
        return 1;
    }

    // 2. Load the dynamic library into memory
    HMODULE dllHandle = LoadLibraryA(dllPath);
    if (!dllHandle) {
        std::cerr << "[Host] Failed to load DLL. Error code: "
            << GetLastError() << "\n";
        return 1;
    }

    // 3. Version handshake check
    auto getVersion = reinterpret_cast<GetPluginAPIVersionFn>(
        GetProcAddress(dllHandle, "GetPluginAPIVersion")
        );

    if (!getVersion || getVersion() != ENGINE_ABI_VERSION) {
        std::cerr << "[Host] ABI version mismatch or invalid plugin!\n";
        FreeLibrary(dllHandle);
        return 1;
    }

    // 4. Resolve remaining lifecycle exports
    auto getName = reinterpret_cast<GetPluginNameFn>(
        GetProcAddress(dllHandle, "GetPluginName")
        );
    auto createPlugin = reinterpret_cast<CreatePluginFn>(
        GetProcAddress(dllHandle, "CreatePlugin")
        );
    auto destroyPlugin = reinterpret_cast<DestroyPluginFn>(
        GetProcAddress(dllHandle, "DestroyPlugin")
        );

    if (!createPlugin || !destroyPlugin) {
        std::cerr << "[Host] Missing lifecycle entry points in DLL.\n";
        FreeLibrary(dllHandle);
        return 1;
    }

    std::cout << "[Host] Loading module: "
        << (getName ? getName() : "Unnamed") << "\n";

    // 5. Instantiate and initialize plugin
    IEnginePlugin* pluginInstance = createPlugin();
    if (pluginInstance) {
        PluginContext ctx{ .dummyData = nullptr };

        if (pluginInstance->OnLoad(ctx)) {
            // Test a frame update
            pluginInstance->OnUpdate(0.016f);
        }

        // 6. Shutdown and unload in reverse order
        pluginInstance->OnUnload();
        destroyPlugin(pluginInstance);
    }

    // 7. Release the DLL file lock
    FreeLibrary(dllHandle);
    std::cout << "[Host] Module successfully unloaded. Exiting.\n";

    return 0;
}