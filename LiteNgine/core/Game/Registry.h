#pragma once
#include <cstdint>
namespace lte {
	class Registry
	{
		using Slate = uint32_t;
		Slate NewSlate();
	};
}
