#include "PluginScanner.h"
#include <filesystem>
#include <nlohmann/json.hpp>
namespace ltCore
{
	using json = nlohmann::json;

	using fs = std::filesystem;
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

	}
}