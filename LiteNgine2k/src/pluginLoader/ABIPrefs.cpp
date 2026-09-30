#include "ABIPrefs.h"
#include "../forScrap/Lt_Console.h"
PrefHook* ltCore::preferencesDelegate::createInterface(std::string name)
{
    auto it = Hooks.find(name);
    if (it != Hooks.end())
    {
        lte::Con::LogError("Attempted to create plugin interface under existing plugin name! may be a duplicaion or security risk!", HIGH_SEVERITY, TAG_ENGINE);
        return nullptr;
    }
    PrefHook* hook = new PrefHook{};
    hook->name = name;
    Hooks[name] = hook;
    return hook;
}

void ltCore::preferencesDelegate::Shutdown()
{
    PrefHook::clearcache();
    for (auto& port : Hooks)
    {
        delete(port.second);

    }
    Hooks.clear();
}

void ltCore::preferencesDelegate::Startup()
{

}