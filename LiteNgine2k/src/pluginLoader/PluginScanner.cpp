#include "PluginScanner.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <fstream>
namespace ltCore
{
	using json = nlohmann::json;

	namespace fs = std::filesystem;
	void PluginScanner::Scan()
	{
		std::vector<fs::path> pathes;
		for (const auto& entry : fs::recursive_directory_iterator(pluginPath))
		{
			//recursive scrape
			if (entry.is_regular_file())
			{
				if (entry.path().extension() == ".LiteMeta")
				{
					pathes.push_back(entry.path());
				}
			}

		}
		//then here it loads all 
		for (auto& path : pathes)
		{

			std::ifstream f(path);
			json data = json::parse(f);
			
		}
	}
}