#pragma once
#include <filesystem>
namespace lte
{
	namespace fs = std::filesystem; 
	class MistMaker
	{

	public:
		enum class FileType
		{
			Model,
			Audio,
			Texture,
			Misc,
			LuaScript,
			GraphScript,
			Slate,
			Config,
			Data,
		};

		static void generateMistFile(fs::path filepath);
	private:
		static bool CheckImage(std::string& filepath, int* width, int* height);
	};
}