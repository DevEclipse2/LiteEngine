// LiteNgine2k.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "src/Application.h"
#include <iostream>
#include "src/forScrap/Lt_Console.h"
int main() {
    ltCore::Application app{};
    try {
        app.run();
    }
    catch (const std::exception& e) {
        //crash
        lte::Con::LogError(e.what(), FATAL_SEVERITY, TAG_ENGINE);
        lte::Con::OutputFile();
        std::cerr << e.what() << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }
    std::cin.get(); // pause before program closes 
    return EXIT_SUCCESS;
    return 0;
}