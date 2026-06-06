#include "Bootstrapper.h"
namespace lte {

	std::string Bootstrapper::preferencesFileName = "LiteNginePref.ini";
	void Bootstrapper::OnWake(uint8_t* result)
	{
		Con::Log("begin launch bootstrap process", LOG_INFORMATIONAL);
		std::string input;
		choice:
		std::cout << "Welcome to LiteNgine \n you may specify launch parameters here or change preferences without launching the engine. \n you may type:" << std::endl;
		std::cout << "'continue'	to continue launch with previous session's preferences " << std::endl;
		std::cout << "'load'		to load different preferences from last session" << std::endl;
		std::cout << "'modify'		to modify launch parameters" << std::endl;
		std::cout << "'addarg'		to add special parameters" << std::endl;
		std::cout << "'reset'		to reset all preferences" << std::endl;
		std::cout << "'exit'		to abort launch of liteNgine :[ " << std::endl;
		std::cin >> input;
		Con::Log("user entered selection", LOG_INFORMATIONAL);

		if (input == "continue")
		{
			Con::Log("launch bootstrap process completed with response : continue", LOG_INFORMATIONAL);
			*result = 0;
		}
		else if (input == "reset")
		{
			Con::Log("launch bootstrap proceeding with operation : reset preferences", LOG_INFORMATIONAL);
			//make choice choose save
			chooseifPreserve:
			std::cout << "would you like to save current preferences as a new file in a different location? \n Type Y/N " << std::endl;
			std::cin >> input;
			if (input == "Y" || input == "y")
			{
				std::cout << "user choice confirmed and acknowledged. enter new file name:" << std::endl;
				Con::Log("user chose to save original preferences", LOG_INFORMATIONAL);

				std::ifstream file(preferencesFileName);

				if (!file.is_open())
				{
					std::cerr << "unable to open original preferences file" << std::endl;
					Con::Log("failed to open original preferences file", LOG_LOW_SEVERITY);
				}
				std::string newName;
				std::cin >> newName;
				Con::Log("user saved original file to path " + newName, LOG_INFORMATIONAL);
				std::ofstream outFile(newName);
				if (outFile.is_open())
				{
					outFile << file.rdbuf();
					outFile.close();
				}
				else {
					std::cerr << "Could not open and write to the file." << std::endl;
					Con::Log("failed to write new preferences to file", LOG_LOW_SEVERITY);

				}

			}
			else if (input == "N" || input == "n")
			{
				std::cout << "user selection confirmed and acknowledged." << std::endl;
				Con::Log("user chose to discard preferences", LOG_INFORMATIONAL);

			}
			else
			{
				std::cout << "unknown input, please try again." << std::endl;
				Con::Log("unknown input", LOG_INFORMATIONAL);
				goto chooseifPreserve;
			}

			std::ofstream outFile(preferencesFileName);
			if (outFile.is_open())
			{
				outFile << GenerateFile();
				outFile.close();
			}
			else
			{
				std::cerr << "Could not copy preferences to the main file." << std::endl;
				Con::Log("failed to copy to main file", LOG_LOW_SEVERITY);

			}
			std::cout << "Preferences Reset Sucessfully" << std::endl;
			std::cout << "Operation complete, returning to main..." << std::endl;
			Con::Log("preferences reset successfully", LOG_INFORMATIONAL);
			goto choice;
		}
		else if (input == "load")
		{
			Con::Log("launch bootstrap proceeding with operation : load preferences", LOG_INFORMATIONAL);
			std::cout << "please enter the file path of the designated file" << std::endl;
			std::string path;
			std::cin >> path;
			std::ifstream newFile(path);
			
			if (newFile.is_open()) {
				std::cout << "new preferences found successfully" << std::endl;
				Con::Log("launch bootstrap preferences found successfully at :" + path, LOG_INFORMATIONAL);
			}
			else
			{
				std::cerr << "unable to open new preferences file" << std::endl;
				Con::Log("launch bootstrap preferences was not found with filepath : " + path, LOG_LOW_SEVERITY);

			}
			chooseIfSave:
			std::cout << "would you like to save current preferences as a new file in a different location? \n Type Y/N " << std::endl;
			std::cin >> input;
			if (input == "Y" || input == "y")
			{
				std::cout << "user choice confirmed and acknowledged. enter new file name:" << std::endl;
				Con::Log("user chose to save original preferences" , LOG_INFORMATIONAL);

				std::ifstream file(preferencesFileName);
				
				if (!file.is_open())
				{
					std::cerr << "unable to open original preferences file" << std::endl;
					Con::Log("failed to open original preferences file", LOG_LOW_SEVERITY);
				}
				std::string newName;
				std::cin >> newName;
				Con::Log("user saved original file to path " + newName, LOG_INFORMATIONAL);
				std::ofstream outFile(newName);
				if (outFile.is_open()) 
				{
					outFile << file.rdbuf();
					outFile.close();
				}
				else {
					std::cerr << "Could not open and write to the file." << std::endl;
					Con::Log("failed to write new preferences to file", LOG_LOW_SEVERITY);

				}


			}
			else if (input == "N" || input == "n")
			{
				std::cout << "user selection confirmed and acknowledged." << std::endl;
				Con::Log("user chose to discard preferences", LOG_INFORMATIONAL);

			}
			else
			{
				std::cout << "unknown input, please try again." << std::endl;
				Con::Log("unknown input", LOG_INFORMATIONAL);
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
				Con::Log("failed to copy to main file", LOG_LOW_SEVERITY);

			}
		}
		else if (input == "exit")
		{
			*result = 1;
			Con::Log("launch bootstrap process terminated with response : abort launch", LOG_INFORMATIONAL);
		}
		else if (input == "modify")
		{

		}
		else if (input == "addarg")
		{

		}
		else
		{
			Con::Log("launch bootstrap process failed with result : unidentified input , retrying...", LOG_LOW_SEVERITY);
			std::cout << "|unknown input, please try again or use HELP_ME_PLEASE_AAAAA in order to bring up the 300 billion parameter local llm that comes prepackaged|" << std::endl;
			Con::Log("retrying", LOG_INFORMATIONAL);
			goto choice;
		}
	}
	std::string Bootstrapper::GenerateFile()
	{
		std::string newFile = "";
		Insert(newFile, "[Graphics]");
		Insert(newFile, "main_window_w = 800");
		Insert(newFile, "main_window_h = 600");
		Insert(newFile, "vsync = true");
		Insert(newFile, "multisampling = true");
		Insert(newFile, "antialiasing = true");
		Insert(newFile, "frames_in_flight = 3");
		Insert(newFile, "use_vulkan = true");
		Insert(newFile, "vulkan_version = 14");
		Insert(newFile, "[debug]");
		Insert(newFile, "debug = true");
		Insert(newFile, "log_filepath = logs/");
		Insert(newFile, "console_filter_level = none");

		Insert(newFile, "[performance]\ngpu_compute = false\nmultithreading = false\n");
		
		Insert(newFile, "[profiling]\nuse_profiler = true\ncpu_profiler = true\ngpu_profiler = true\nshow_fps = false\nmanual_begin = true\nwrite_profiler_file = true\nprofiler_res_directory = iridium_profiler/ ");
		
		Insert(newFile, "[addon]\nuse_addons = true\nsafety_level = 1\n; safety level 0 = official addons 1 = endorsed addons 2 = managed installed addons 3 = unknown import addons\nsafemode = false\nrecovery = false");

		Insert(newFile, "[special]\n; if you need to specify anything you want, do it here");


		return newFile;
	}
	void Bootstrapper::Insert(std::string& str, std::string information)
	{
		str += information + "\n";
	}
}
