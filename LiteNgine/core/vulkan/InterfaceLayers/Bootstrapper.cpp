#include "Bootstrapper.h"
namespace lte {

	std::string Bootstrapper::preferencesFileName = "LiteNginePref.ini";
	void Bootstrapper::OnWake(uint8_t* result)
	{
		std::string input;
		std::cout << "Welcome to LiteNgine \n you may specify launch parameters here or change preferences without launching the engine. \n you may type:" << std::endl;
		std::cout << "'continue'	to continue launch with previous session's preferences " << std::endl;
		std::cout << "'reset'		to reset all preferences" << std::endl;
		std::cout << "'load'		to load different preferences from last session" << std::endl;
		std::cout << "'exit'		to abort launch of liteNgine :[ " << std::endl;
		std::cin >> input;
		if (input == "continue")
		{
			*result = 0;
		}
		else if (input == "reset")
		{

		}
		else if (input == "load")
		{
			std::cout << "please enter the file path of the designated file" << std::endl;
			std::string path;
			std::cin >> path;
			std::ifstream newFile(path);
			
			if (newFile.is_open()) {
				std::cout << "new preferences found successfully" << std::endl;
			}
			else
			{
				std::cerr << "unable to open new preferences file" << std::endl;
			}
			chooseIfSave:
			std::cout << "would you like to save current preferences as a new file in a different location? \n Type Y/N " << std::endl;
			std::cin >> input;
			if (input == "Y" || input == "y")
			{
				std::cout << "user choice confirmed and acknowledged. enter new file name:" << std::endl;

				std::ifstream file(preferencesFileName);
				
				if (!file.is_open())
				{
					std::cerr << "unable to open original preferences file" << std::endl;
				}
				std::string newName;
				std::cin >> newName;
				std::ofstream outFile(newName);
				if (outFile.is_open()) 
				{
					outFile << file.rdbuf();
					outFile.close();
				}
				else {
					std::cerr << "Could not open and write to the file." << std::endl;
				}

			}
			else if (input == "N" || input == "n")
			{
				std::cout << "user selection confirmed and acknowledged." << std::endl;
			}
			else
			{
				std::cout << "unknown input, please try again." << std::endl;
				goto chooseIfSave;
			}
			std::ofstream outFile(preferencesFileName);
			if (outFile.is_open())
			{
				outFile << newFile.rdbuf();
				outFile.close();
			}
			else 
			{
				std::cerr << "Could not copy preferences to the main file." << std::endl;
			}
		}
		else if (input == "exit")
		{
			*result = 1;
		}
		else
		{
			std::cout << "|unknown input, please try again or use HELP_ME_PLEASE_AAAAA in order to bring up the 300 billion parameter local llm that comes prepackaged|" << std::endl;
		}
	}
}
