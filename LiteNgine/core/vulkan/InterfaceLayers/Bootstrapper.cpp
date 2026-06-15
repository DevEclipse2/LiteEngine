#include "Bootstrapper.h"
namespace lte {

	std::string Bootstrapper::preferencesFileName = "LiteNginePref.ini";
	std::map<std::string, std::map< std::string, Preference>>  Bootstrapper::data;
	std::map<uint16_t, std::string> Bootstrapper::comments;

	

	void Bootstrapper::OnWake(uint8_t* result)
	{
		LoadPrefs(preferencesFileName);
		Con::Log("begin launch bootstrap process", LOG_INFORMATIONAL);
		std::string input;
		
		std::cout << "Welcome to LiteNgine \n you may specify launch parameters here or change preferences without launching the engine. \n you may type:" << std::endl;
		std::cout << "'continue'	to continue launch with previous session's preferences " << std::endl;
		std::cout << "'load'		to load different preferences from last session" << std::endl;
		std::cout << "'modify'		to modify launch parameters" << std::endl;
		std::cout << "'addarg'		to add special parameters" << std::endl;
		std::cout << "'reset'		to reset all preferences" << std::endl;
		std::cout << "'exit'		to abort launch of liteNgine :( " << std::endl;
		bool command = false;
		while (!command)
		{
			std::cin >> input;
			Con::Log("user entered selection", LOG_INFORMATIONAL);
			if (input == "continue")
			{
				Con::Log("launch bootstrap process completed with response : continue", LOG_INFORMATIONAL);
				*result = 0;
				command = true;
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
			}
			else if (input == "load")
			{
			loadprocess:
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
					goto loadprocess;
				}
			chooseIfSave:
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
						goto chooseIfSave;

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
						std::cerr << "Retrying..." << std::endl;
						Con::Log("failed to write new preferences to file", LOG_LOW_SEVERITY);
						goto chooseIfSave;

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
				command = true;

			}
			else if (input == "modify")
			{
				Con::Log("launch bootstrap process proceeding with response : modify value", LOG_INFORMATIONAL);
				std::cout << "List of available key-value pairs are as follows : " << std::endl;
				for (const auto& [key, value] : data) {
					// key and value are read-only references
					std::cout << key << std::endl;
					for (const auto& [keyinner, valueinner] : value)
					{
						// key and value are read-only references
						//this code makes each line of certain length
						uint8_t strLength = 40 - keyinner.length();
						if (strLength > 40)
						{
							strLength = 0;
						}
						std::string str(strLength, ' ');

						std::cout << keyinner << str << valueinner.value << std::endl;
					}
				}
				bool proceed = false;
				std::map<std::string, std::map<std::string, Preference>> newprefs = data;
				std::cout << "use \" help \" to see available commands" << std::endl;
				std::string category = "uuddlrlrbaStart";
				std::vector<std::tuple<std::string, std::string,Preference>> preferenceHistory ;
				std::vector<Preference> preferenceoriginal;
				while (!proceed)
				{
					std::string choice;
					std::getline(std::cin, choice);
					Con::Log("user choice :" + choice, LOG_INFORMATIONAL);

					if (choice == "help")
					{
						std::cout << "available commands:" << std::endl;
						std::cout << "'apply'				to apply all changed entries" << std::endl; //done
						std::cout << "'history'				to see all changed entries" << std::endl;
						std::cout << "'revert 00'			to discard changed entry at index" << std::endl;
						std::cout << "'restore'				to restore engine entries to defaults; leaves custom values unchanged" << std::endl;
						std::cout << "'cat [category_name]'	to change scope to that category" << std::endl;
						std::cout << "'exit'				to reset category or finish modifying values" << std::endl;
						std::cout << "'listkv'				to list all key value pairs within that category" << std::endl;
						std::cout << "'listcat'				to list all categories" << std::endl;
						std::cout << "type the name of desired key to enter a new value" << std::endl;

					}
					else if (choice == "exit")
					{
						if (category == "uuddlrlrbaStart")
						{
							std::cout << "Warning : exiting does not save your modifications. call 'save' to do so now \n 'cancel' cancels this operation and \n 'continue' to go through with this operation" << std::endl;
							std::string exitchoice;
							std::cin >> exitchoice;
							if (exitchoice == "save")
							{
								std::cout << "applying key value pairs and saving to hard drive..." << std::endl;
								data = newprefs;
								std::fstream outFile(preferencesFileName);
								if (outFile.is_open())
								{
									outFile << SaveFile();
									outFile.close();
								}
								proceed = true;
							}
							else if (exitchoice == "cancel")
							{
								std::cout << "returning to main modify tab..." << std::endl;

							}
							else if (exitchoice == "continue")
							{
								std::cout << "exiting..." << std::endl;
								proceed = true;
							}
							else 
							{
								std::cout << "unrecognised input, returning to modify tab" << std::endl;
							}
						}
						else
						{
							category = "uuddlrlrbaStart";
						}
					}
					else if (choice == "apply")
					{
						std::cout << "applying key value pairs..." << std::endl;
						data = newprefs;
					}
					else if (choice == "history")
					{

					}
					else if (choice.compare(0, 3, "cat") == 0)
					{
						std::string catstring = choice;
						while (catstring.at(0) != '[')
						{
							if (catstring.size() == 0)
							{
								std::cout << "error : specified category must be present and bracketed in '[' ']' characters" << std::endl;
								Con::Log("missing category" + choice, LOG_LOW_SEVERITY);
								break;
							}
							catstring.erase(0, 1);
						}

						if (newprefs.find(catstring) != newprefs.end())
						{
							category = catstring;
							std::cout << "switched selected category" << std::endl;
						}
						else
						{
							std::cout << "no matching category found" << std::endl;
						}
					}
					else if (choice[0] == 'r' && choice[2] == 'v')
					{
						//revert

					}
					else if (choice == "listkv")
					{
						if (category != "uuddlrlrbaStart")
						{
							for (const auto& [keyinner, valueinner] : newprefs[category])
							{
								// key and value are read-only references
								//this code makes each line of certain length
								uint8_t strLength = 40 - keyinner.length();
								if (strLength > 40)
								{
									strLength = 0;
								}
								std::string str(strLength, ' ');

								std::cout << keyinner << str << valueinner.value << std::endl;
							}
						}
						else
						{
							std::cout << " please select a category first! \nuse 'listcat' to see all categories and 'cat [category-name]' to select it" << std::endl;
						}
					}
					else if (choice == "listcat")
					{
						for (const auto& [keyinner, valueinner] : newprefs)
						{
							std::cout << keyinner << std::endl;
						}
					}
					else if (category != "uuddlrlrbaStart")
					{
						if (choice.length() == 0) {
							continue;
						}
						//trims whitespace
						while (choice.at(choice.size() - 1) == ' ')
						{
							if (choice.length() == 0) {
								continue;
							}
							choice.erase(choice.size() - 1, 1);
						}
						if (newprefs[category].find(choice) != newprefs[category].end())
						{
							std::cout << "currently selected key : " << choice << "\t with current value :\t" << newprefs[category][choice].value << " under category : \"" << category << "\"" << '\n' << "enter new value :" << std::endl;
							std::string newVal;
							std::cin >> newVal;
							newprefs[category][choice].value = newVal;
							Con::Log("currently selected key : " + choice + "\t with current value :\t" + newprefs[category][choice].value + " under category : \"" + category + "\"" + '\n' + "changed to : " + newVal, LOG_INFORMATIONAL);
							std::cout << "new value saved!" << std::endl;
						}
						else
						{
							std::cout << "unknown command : \"" << choice << "\" entered; use \" help \" to see available commands" << std::endl;
							Con::Log("unknown command entered : " + choice, LOG_LOW_SEVERITY);
						}
					}
					else
					{
						std::cout << "unknown command : \"" << choice << "\" entered; use \" help \" to see available commands" << std::endl;
						Con::Log("unknown command entered : " + choice, LOG_LOW_SEVERITY);
					}
				}
			}
			else if (input == "addarg")
			{

			}
			else if (input == "help")
			{
				std::cout << "'continue'	to continue launch with previous session's preferences " << std::endl;
				std::cout << "'load'		to load different preferences from last session" << std::endl;
				std::cout << "'modify'		to modify launch parameters" << std::endl;
				std::cout << "'addarg'		to add special parameters" << std::endl;
				std::cout << "'reset'		to reset all preferences" << std::endl;
				std::cout << "'exit'		to abort launch of liteNgine :( " << std::endl;
			}
			else
			{
				Con::Log("launch bootstrap process failed with result : unidentified input , retrying...", LOG_LOW_SEVERITY);
				std::cout << "|unknown input, please try again or use HELP_ME_PLEASE_AAAAA in order to bring up the 300 billion parameter local llm that comes prepackaged|" << std::endl;
				Con::Log("retrying", LOG_INFORMATIONAL);
			}
		}

	}
	void Bootstrapper::LoadPrefs(std::string filename)
	{

		std::ifstream file(filename);
		if (!file.is_open()) 
		{
			std::cout << "No Preferences file found,generating new file..." << std::endl;
			Con::Log("no preference file found : generating new", LOG_LOW_SEVERITY);
			std::ofstream outFile(preferencesFileName);
			if (outFile.is_open())
			{
				outFile << GenerateFile();
				outFile.close();
			}
			std::ifstream file(filename);
			if (!file.is_open())
			{
				std::cout << "unable to generate required file. please contact developer" << std::endl;
				Con::Log("unable to generate or load preferences file", LOG_CRIT_SEVERITY);
				std::ofstream outFile(preferencesFileName);
				if (outFile.is_open())
				{
					outFile << GenerateFile();
					outFile.close();
				}
				std::ifstream file(filename);

			}
		}

		std::string line;
		uint16_t linecount = 0;
		std::string subsectName = "";
		while (std::getline(file, line)) {
			// Remove whitespace/newlines
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
			if (line.empty())continue; // Skip empty or comments
			switch (line[0])
			{
			case ';':
				comments[linecount] = line.substr(1);
				continue;
			case '[':
				//this is a new subsection
				subsectName = line;
				continue;
			default:
				break;
			}

			size_t delimiterPos = line.find('=');
			if (delimiterPos != std::string::npos) {
				std::string key = line.substr(0, delimiterPos);
				while (key.at(key.size() - 1) == ' ')
				{
					key.erase(key.size() - 1, 1);
				}
				std::string value = line.substr(delimiterPos + 1);
				Preference loadedPref;
				loadedPref.value = value;
				loadedPref.lineNum = linecount;
				data[subsectName][key] = loadedPref;
			}
			linecount++;
		}
	}
	std::string Bootstrapper::SaveFile()
	{
		Con::Log("writing new preferences file to disk...", LOG_INFORMATIONAL);
		std::string outputstring = "";
		std::vector<std::string> lines;
		std::vector<flattenedData> flat_list;
		for (const auto& cat_pair : data) {
			const std::string& category_name = cat_pair.first;
			for (const auto& pref_pair : cat_pair.second) {
				flat_list.push_back({ category_name, pref_pair.first, pref_pair.second });
			}
		}

		std::sort(flat_list.begin(), flat_list.end(),
			[](const flattenedData& a, const flattenedData& b) {
				return a.preference.lineNum < b.preference.lineNum;
			}
		);

		std::string current_category = "";
		for (const auto& item : flat_list) {
			// If the category changes, we print the category header.
			// Because the list is sorted by line number, this automatically places
			// the categories exactly where they belong in the sequence.
			if (item.category != current_category) {
				lines.emplace_back( "[" + item.category + "]");
				current_category = item.category;
			}
			lines.emplace_back(item.key + " = " + item.preference.value);
			// Output the actual preference
		}
		//inserts comments in between like it should
		for (const auto& [key, value] : comments)
		{
			lines.insert(lines.begin() + key, value);
		}
		for(std::string substr : lines)
		{
			outputstring += substr + "\n";
		}
		return outputstring;
	}
	std::string Bootstrapper::GenerateFile()
	{
		std::string newFile = "";
		Insert(newFile, "[graphics]");
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
