#include "Lt_Importer.h"
#define Load_Success 0
#define Load_Fail_Generic 1
#define Load_Fail_UnsupportedFile	2
#define Load_Fail_UnparsableFormat	3
#define Load_Fail_CorruptedFile		4
// Assimp headers
#include <assimp/Importer.hpp>
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

namespace lte 
{
	uint8_t Lt_Importer::Load(const std::string& path, unsigned int pFlags)
	{
		if (pFlags == 0) 
		{
			//no flags provided
			pFlags = aiProcess_CalcTangentSpace |
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices |
				aiProcess_SortByPType;
		}
		Assimp::Importer importer;
		const aiScene* importedScene = importer.ReadFile(path,pFlags);
		if (importedScene == nullptr) 
		{
			std::string errstr = importer.GetErrorString();
			Con::LogFailure(errstr + "Assimp loading of model failed from internal engine class Lt_Importer", HIGH_SEVERITY,TAG_ENGINE);
			return 1;
		}
		return 0;

	}
}