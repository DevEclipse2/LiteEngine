#include "ShaderLoader.h"
namespace lte {
	ShaderLoader::ShaderLoader() {

	}
    ShaderLoader::~ShaderLoader() {

    }
    void ShaderLoader::UpdateShaders() {

    }
    std::vector<char> ShaderLoader::readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        std::vector<char> buffer(file.tellg());
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        file.close();

        return buffer;
    }
}