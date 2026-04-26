
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

//idk why but nothing runs without this

int main()
{
    lte::main app{};
    try{
        app.run();
    }
    catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
Severity	Code	Description	Project	File	Line	Suppression State
Error	LNK2019	unresolved external symbol __imp_DispatchMessageW referenced in function _glfwInitWin32	LiteNgine	C:\git\LiteEngine\LiteNgine\glfw3.lib(win32_init.obj)	1


*/