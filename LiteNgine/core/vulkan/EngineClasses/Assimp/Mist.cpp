#include "Mist.h"
#include "../Lt_Console.h"
#include <fstream>
#include "Lt_Importer.h"
#include "stb_image.h"
namespace lte {
	void lte::MistMaker::generateMistFile(fs::path filepath)
	{

		if (fs::is_directory(filepath))
		{
			Con::LogError("Cannot Generate .mist file for directory!", LOW_SEVERITY, TAG_ENGINE);
			return;
		}
		//takes the asset file and first sees what the hell it is
		FileType type = FileType::Misc;
		std::string pathstr = filepath.string();
		int width = 0;
		int height = 0;
		if (filepath.has_extension() && filepath.extension() == ".lua")
		{
			type = FileType::LuaScript;
		}
		else if (Lt_Importer::CheckIsValid(pathstr))
		{
			type = FileType::Model;
		}
		else if(CheckImage(pathstr,&width,&height))
		{
			//...
			type = FileType::Texture;
		}
		std::ofstream outMist(filepath / ".mist");

		if (!outMist.is_open())
		{
			Con::LogError("Unable to open file!", HIGH_SEVERITY, TAG_ENGINE);
			return;
		}
		switch (type)
		{
		case FileType::LuaScript:

			outMist << "luaScript" << std::endl;
			break;
		case FileType::Model:
			outMist << "Model" << std::endl;
			break;
		case FileType::Texture:
			outMist << "Texture" << std::endl;
			outMist << "width:" << width << std::endl;
			outMist << "height:" << height << std::endl;
			break;

		}
		outMist.close();
	}

	bool MistMaker::CheckImage(std::string& filepath, int* width, int* height)
	{
		int channels = 0;
		uint8_t* pixels = stbi_load(filepath.c_str(), width, height, &channels, STBI_rgb_alpha);
		if (!pixels)
		{
			return false;
		}
		return true;
	}

}
