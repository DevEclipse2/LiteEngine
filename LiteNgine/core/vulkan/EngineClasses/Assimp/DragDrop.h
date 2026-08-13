#pragma once

#include <vector>
#include <string>
#include <iostream>
namespace lte
{
	class DragDrop
	{
	public:
		static void OnFilesDropped(const std::vector<std::string>& paths);
	};
}