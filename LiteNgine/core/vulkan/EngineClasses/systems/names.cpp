#include "names.h"
#include <algorithm>
#include "../Lt_Console.h"
namespace lte {
	void names::RegisterName(std::string& Name, uint32_t SlateId, char useBits)
	{
		auto it = slateToIndexMap.find(SlateId);

		if (it != slateToIndexMap.end())
		{
			// ID is already registered, just overwrite the data
			Con::Log("slateid already in use, modifying present values", TAG_ENGINE);

			uint32_t index = it->second; // Get the array index from the map

			nameComponents[index].name = Name;
			nameComponents[index].usebits = useBits | in_use;
			return;
		}
		slateName slate;
		slate.name = Name;
		slate.usebits = useBits | in_use;
		if (!freeSpaces.empty())
		{
			uint32_t freeIndex = freeSpaces.back();
			freeSpaces.pop_back();
			nameComponents[freeIndex] = slate;
			AssignedSlates[freeIndex] = SlateId;
			slateToIndexMap[SlateId] = freeIndex;
		}
		else
		{
			nameComponents.push_back(slate);
			AssignedSlates.push_back(SlateId);
			slateToIndexMap[SlateId] = freeIndex;
		}
	}
	void names::RemoveName(uint32_t SlateId)
	{
		if (SlateId == static_cast<uint32_t>(-1))
		{
			Con::LogError("invalid slateid (-1)", MED_SEVERITY, TAG_ENGINE);
		}
		auto it = slateToIndexMap.find(SlateId);

		if (it != slateToIndexMap.end())
		{
			Con::Log("found, tagging for overwrite", TAG_ENGINE);
			uint32_t index = it->second;
			freeSpaces.push_back(index);
			AssignedSlates[index] = static_cast<uint32_t>(-1);
			nameComponents[index].usebits &= ~in_use;
			slateToIndexMap.erase(it);
		}
	}
}
