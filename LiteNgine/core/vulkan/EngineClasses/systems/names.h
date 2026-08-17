#pragma once
#include <string>
#include <vector>
#define in_use 1
#include <WinNls.h>
#include <stringapiset.h>
namespace lte
{
	struct slateName
	{
		std::string name;
		char usebits;
	};
	class names
	{
	public:
		inline static std::vector<slateName> nameComponents;
		inline static std::vector<uint32_t> AssignedSlates;
		inline static std::vector<uint32_t> freeSpaces;

		inline static std::unordered_map<uint32_t, uint32_t> slateToIndexMap;

		static void RegisterName(const std::string& Name, uint32_t SlateId, char useBits);
		static void RemoveName(uint32_t SlateId);
	};
}


