#pragma once
#include <string>
#include <vector>
#define in_use 1
namespace lte
{
	struct slateName
	{
		std::wstring name;
		uint32_t slateId;
		char usebits;
	};
	class names
	{
		std::vector<slateName> nameComponents;
	};
}


