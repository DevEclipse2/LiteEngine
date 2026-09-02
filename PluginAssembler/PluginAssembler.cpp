// PluginAssembler.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
int main()
{
    std::string inputbuf;
    std::cout << "Welcome to the liteNgine Assembler!\n This program is intended to assist the user in assembling their plugins for liteNgine\n";
    std::cout << "This program can help the user sort out dependancies for their plugin, as well as generating the .LiteMeta manifest for the plugins!\n";
    std::cout << "The assembler also helps users add additional binaries dependancies for multiple Addons to share\n";
    std::cout << "To begin, make sure all files are placed within clean directories, as all files will be treated as dependancies, even the ones that contain your childhood photos and the funny numbers at the back of your credit card \n";
    std::cout << "Commands Available : " << std::endl;
    std::cout << "help              : lists available commands, add help to the back of other commands to get specific tips" << std::endl;
    std::cout << "about             : what is it about anyways?" << std::endl;
    std::cout << "generateTemplate  : allows user to input path to a .LiteMeta file that acts as a template for the new one" << std::endl;
    std::cout << "manidoc           : runs a quick check and diagnosis of problematic .LiteMeta file" << std::endl;
    std::cout << "fixer             : attempts repair of problematic .LiteMeta file " << std::endl;
    std::cout << "builder           : the actual system for the plugin" << std::endl;

    std::cout << "Input the file directory below : " << std::endl;
    std::cin >> inputbuf;
    std::cout << "Would you like to generate a pack or a manifest?" << std::endl;
    //std::cout << "" << std::endl;
}
//need to include the plugin abi to ask for the version