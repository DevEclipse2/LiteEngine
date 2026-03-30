#pragma once
#include <fstream>
#include <vector>
namespace lte {


	class ShaderLoader
	{
	public:
		ShaderLoader();
		~ShaderLoader();
		void UpdateShaders();
		std::vector<char> readFile(const std::string& filename);
	private:
	};
}

