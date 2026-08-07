
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>
#include "App.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include "EngineClasses/Lt_Console.h"
//idk why but nothing runs without this

int main()
{
    lte::main app{};
    try{
        app.run();
    }
    catch (const std::exception& e){
        //crash
        lte::Con::LogError(e.what(), FATAL_SEVERITY, TAG_ENGINE);
        lte::Con::OutputFile();
        std::cerr << e.what() << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }
    std::cin.get(); // pause before program closes 
    return EXIT_SUCCESS;
}

/*
Severity	Code	Description	Project	File	Line	Suppression State
Error	LNK2019	unresolved external symbol __imp_DispatchMessageW referenced in function _glfwInitWin32	LiteNgine	C:\git\LiteEngine\LiteNgine\glfw3.lib(win32_init.obj)	1


*/