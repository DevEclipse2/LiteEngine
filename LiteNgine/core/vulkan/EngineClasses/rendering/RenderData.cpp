#include "RenderData.h"
#include "../../Reworked/Buffers.h"
#include "../Preferences.h"
#include "../Lt_Console.h"
namespace lte {
	void RenderData::createBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device, RenderData::BufferType type)
	{
		//assert(vertexBuffer == nullptr);
		vk::DeviceSize ByteSize = -1;
		vk::BufferUsageFlagBits flagBit;
		switch (type)
		{
		case RenderData::BufferType::GenericBuffer:
			Con::LogError("GenericBuffer is an invalid buffer type used to designate incorrectly formed buffers and is not a catch all,do not create a buffer with type genericBuffer!", HIGH_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::VertexBuffer:
			ByteSize = Preferences::Optimiser::VertexBufferSize * sizeof(Vertex);
			flagBit = vk::BufferUsageFlagBits::eVertexBuffer;
			break;
		case RenderData::BufferType::SkinnedVertexBuffer:
			ByteSize = Preferences::Optimiser::SkinnedVertexBufferSize * sizeof(skinnedVertex);
			flagBit = vk::BufferUsageFlagBits::eVertexBuffer;
			break;
		case RenderData::BufferType::IndiceBuffer:
			ByteSize = Preferences::Optimiser::IndexBufferSize * sizeof(uint32_t);
			flagBit = vk::BufferUsageFlagBits::eIndexBuffer;
			break;
		case RenderData::BufferType::XLVertexBuffer:
			Con::LogError("XLVertexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::XLSkinnedVertexBuffer:
			Con::LogError("XLSkinnedVertexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::XLIndexBuffer:
			Con::LogError("XLIndexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return;
		}
		
		Buffer NewBuffer;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, NewBuffer.buffer, NewBuffer.Allocation, *info.device, device);
		NewBuffer.type = type;
		Buffers.emplace_back(std::make_unique<Buffer>(NewBuffer));
	}
	RenderData::copyResult RenderData::copyBufferContents(BufferType type, void* rawData, uint32_t elementCount, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice, AllocationPosition& allocPos)
	{
		uint32_t DataSize = -1;
		uint32_t TypeBufferMaxSize = -1;
		switch (type)
		{
		case RenderData::BufferType::GenericBuffer:
			Con::LogError("GenericBuffer is an invalid buffer type used to designate incorrectly formed buffers and is not a catch all,do not create a buffer with type genericBuffer!", HIGH_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::VertexBuffer:
			DataSize = sizeof(Vertex);
			TypeBufferMaxSize = Preferences::Optimiser::VertexBufferSize;
			break;
		case RenderData::BufferType::SkinnedVertexBuffer:
			DataSize = sizeof(skinnedVertex);
			TypeBufferMaxSize = Preferences::Optimiser::SkinnedVertexBufferSize;

			break;
		case RenderData::BufferType::IndiceBuffer:
			DataSize = sizeof(uint32_t);
			TypeBufferMaxSize = Preferences::Optimiser::IndexBufferSize;
			break;
		case RenderData::BufferType::XLVertexBuffer:
			Con::LogError("XLVertexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		case RenderData::BufferType::XLSkinnedVertexBuffer:
			Con::LogError("XLSkinnedVertexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;

		case RenderData::BufferType::XLIndexBuffer:
			Con::LogError("XLIndexBuffer is a special buffer type assigned by the engine, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		}
		vk::BufferCreateInfo stagingInfo{};
		if (DataSize == -1)
		{
			Con::LogError("dataSize is -1, this should not happen", CRIT_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		}
		const auto& ByteSize = elementCount * DataSize;
		stagingInfo.size = ByteSize,
		stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc,
		stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(*info.device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		allocPos.size = elementCount;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *info.device, physicalDevice);
		void* dataStaging = stagingBufferMemory.mapMemory(0, ByteSize);
		memcpy(dataStaging, rawData, ByteSize);
		stagingBufferMemory.unmapMemory();

		if (elementCount > TypeBufferMaxSize)
		{
			return RenderData::copyResult::FailureExceedBufferSize;
		}
		for (int i = Buffers.size() - 1; i >= 0; i--)//newest to oldest
		{
			auto& currentBuffer = *Buffers[i];
			if (currentBuffer.type != type)
			{
				//these are not the buffers you are looking for
				if (currentBuffer.type == RenderData::BufferType::GenericBuffer)
				{
					Con::LogError("renderdata : buffertype cannot and should not be GENERIC type, Check Implementation now!", FATAL_SEVERITY, TAG_ENGINE);
				}
				continue;
			}
			if (currentBuffer.FreeSpace.size() == 0)
			{
				if ((TypeBufferMaxSize - currentBuffer.offset) > elementCount)
				{
					//allocate memory at the back
					Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, currentBuffer.offset * DataSize);
					allocPos.bufferId = i;
					allocPos.startindex = currentBuffer.offset;
					currentBuffer.offset += elementCount;
					return RenderData::copyResult::Sucess;

				}
				else{
					continue;
					//check next
				}
			}
			else
			{
				//check memory spot
				uint32_t location = -1;
				uint32_t size = -1;
				uint32_t currentCount = 0;
				uint8_t maxTests = 0;
				uint32_t targIndex = 0;
				for (auto& pair : currentBuffer.FreeSpace)
				{
					//this checks if theres a maximum limit
					if (Preferences::Optimiser::UseDefragLimits && (currentCount > Preferences::Optimiser::maximumSearch || maxTests > Preferences::Optimiser::maximumTrial))
					{
						break;
					}
					if (pair.first >= elementCount && pair.first < size)
					{
						//first is the gap size (in vertex) and second is the index
						//if the space is large enough and also fits better
						targIndex = currentCount;
						size = pair.first;
						location = pair.second;
						maxTests++;
					}
					currentCount++;
				}
				if (size == -1 || location == -1)
				{
					if ((TypeBufferMaxSize - currentBuffer.offset) > elementCount)
					{
						//allocate memory at the back
						Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, currentBuffer.offset * DataSize);
						allocPos.bufferId = i;
						allocPos.startindex = currentBuffer.offset;
						currentBuffer.offset += elementCount;
						return RenderData::copyResult::Sucess;
					}
					else {
						//check next
						continue;
					}
				}
				else
				{
					//copies to index
					Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, location * DataSize);
					//here it reduces the size of the index 			
					allocPos.bufferId = i;
					allocPos.startindex = location;
					if (currentBuffer.FreeSpace[targIndex].first == elementCount) {
						currentBuffer.FreeSpace.erase(currentBuffer.FreeSpace.begin() + targIndex);
					}
					else {
						currentBuffer.FreeSpace[targIndex].first -= elementCount;
						currentBuffer.FreeSpace[targIndex].second += elementCount;
					}
					return RenderData::copyResult::Sucess;
				}
			}

		}
		//allocate new block 
		createBuffer(info,physicalDevice, type);
		allocPos.bufferId = Buffers.size() - 1;
		Buffers::copyBufferIndexed(stagingBuffer, Buffers[Buffers.size()-1]->buffer, ByteSize, info, 0, 0);
		allocPos.startindex = 0;
		return RenderData::copyResult::Sucess;
	}
	RenderData::copyResult RenderData::createXLBuffer(BufferType type, void* rawData, uint32_t elementCount, AllocationPosition& allocPos, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice)
	{
		uint32_t DataSize = -1;
		vk::BufferUsageFlagBits fb;
		switch (type)
		{
		case RenderData::BufferType::GenericBuffer:
			Con::LogError("GenericBuffer is an invalid buffer type used to designate incorrectly formed buffers and is not a catch all,do not create a buffer with type genericBuffer!", HIGH_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::VertexBuffer:
			fb = vk::BufferUsageFlagBits::eVertexBuffer;
			DataSize = sizeof(Vertex);
			break;
		case RenderData::BufferType::SkinnedVertexBuffer:
			fb = vk::BufferUsageFlagBits::eVertexBuffer;
			DataSize = sizeof(skinnedVertex);
			break;
		case RenderData::BufferType::IndiceBuffer:
			fb = vk::BufferUsageFlagBits::eIndexBuffer;
			DataSize = sizeof(uint32_t);
			break;
		case RenderData::BufferType::XLVertexBuffer:
		case RenderData::BufferType::XLSkinnedVertexBuffer:
		case RenderData::BufferType::XLIndexBuffer:
			Con::LogError("XL Buffers are already garanteed for a function called \" createXLBuffer \" DONT DO THIS ", MED_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		}
		vk::BufferCreateInfo stagingInfo{};
		if (DataSize == -1)
		{
			Con::LogError("dataSize is -1, this should not happen", CRIT_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		}
		const auto& ByteSize = elementCount * DataSize;
		stagingInfo.size = ByteSize,
			stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc,
			stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(*info.device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		allocPos.size = elementCount;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *info.device, physicalDevice);
		void* dataStaging = stagingBufferMemory.mapMemory(0, ByteSize);
		memcpy(dataStaging, rawData, ByteSize);
		stagingBufferMemory.unmapMemory();

		Buffer NewBuffer;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferDst | fb, vk::MemoryPropertyFlagBits::eDeviceLocal, NewBuffer.buffer, NewBuffer.Allocation, *info.device, physicalDevice);
		switch (type)
		{
		case RenderData::BufferType::GenericBuffer:
			Con::LogError("GenericBuffer is an invalid buffer type used to designate incorrectly formed buffers and is not a catch all,do not create a buffer with type genericBuffer!", HIGH_SEVERITY, TAG_ENGINE);
			return;
		case RenderData::BufferType::VertexBuffer:
			NewBuffer.type = XLVertexBuffer;
			break;
		case RenderData::BufferType::SkinnedVertexBuffer:
			NewBuffer.type = XLSkinnedVertexBuffer;
			break;
		case RenderData::BufferType::IndiceBuffer:
			NewBuffer.type = XLIndexBuffer;
			break;
		}
		Buffers.emplace_back(std::make_unique<Buffer>(NewBuffer));
		allocPos.bufferId = Buffers.size() - 1;
		Buffers::copyBufferIndexed(stagingBuffer, Buffers[Buffers.size() - 1]->buffer, ByteSize, info, 0, 0);
		allocPos.startindex = 0;
		return RenderData::copyResult::Sucess;
	}
	RenderData::copyResult RenderData::copyBufferContentsBulk(BufferType type, std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> allocPos, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice)
	{
		if (allocPos.empty()) return RenderData::copyResult::Sucess;

		uint32_t DataSize = 0;
		uint32_t MaxCapacity = 0;

		switch (type)
		{
		case RenderData::BufferType::GenericBuffer:
			Con::LogError("GenericBuffer is an invalid buffer type!", HIGH_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		case RenderData::BufferType::VertexBuffer:
			DataSize = sizeof(Vertex);
			MaxCapacity = Preferences::Optimiser::VertexBufferSize;
			break;
		case RenderData::BufferType::SkinnedVertexBuffer:
			DataSize = sizeof(skinnedVertex);
			MaxCapacity = Preferences::Optimiser::SkinnedVertexBufferSize;
			break;
		case RenderData::BufferType::IndiceBuffer:
			DataSize = sizeof(uint32_t);
			MaxCapacity = Preferences::Optimiser::IndexBufferSize;
			break;
		case RenderData::BufferType::XLVertexBuffer:
		case RenderData::BufferType::XLSkinnedVertexBuffer:
		case RenderData::BufferType::XLIndexBuffer:
			Con::LogError("XL buffers are special buffer types, not to be called through this function!", MED_SEVERITY, TAG_ENGINE);
			return copyResult::FailureGeneric;
		default:
			return copyResult::FailureGeneric;
		}

		vk::DeviceSize totalByteSize = 0;
		std::vector<std::tuple<void*, uint32_t, AllocationPosition*>> normalBatches;
		for (const auto& tuple : allocPos)
		{
			void* rawData = std::get<0>(tuple);
			uint32_t elementCount = std::get<1>(tuple);
			AllocationPosition* pos = std::get<2>(tuple);

			if (elementCount > MaxCapacity)
			{
				// Model is a whale. Offload it immediately to the XL allocator.
				pos->IsXL = true;

				// Call your XL buffer function (without the RenderSet argument!)
				copyResult res = createXLBuffer(type, rawData, elementCount, *pos, info, physicalDevice);

				if (res != copyResult::Sucess) {
					Con::LogError("Failed to create XL Buffer for massive model!", HIGH_SEVERITY, TAG_ENGINE);
					return res;
				}
				// XL buffers do not contribute to the bulk staging buffer size.
			}
			else
			{
				// Model is normal. Queue it for the bulk batch.
				pos->IsXL = false;
				normalBatches.push_back(tuple);
				totalByteSize += elementCount * DataSize;
			}
		}
		if (totalByteSize == 0 || normalBatches.empty()) {
			return RenderData::copyResult::Sucess;
		}

		// 3. Allocate ONE giant staging buffer for the normal batch
		vk::raii::Buffer stagingBuffer(nullptr);
		vk::raii::DeviceMemory stagingBufferMemory(nullptr);
		Buffers::createBuffer(
			totalByteSize,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
			stagingBuffer,
			stagingBufferMemory,
			*info.device,
			physicalDevice
		);

		uint8_t* mappedData = static_cast<uint8_t*>(stagingBufferMemory.mapMemory(0, totalByteSize));
		vk::DeviceSize currentStagingByteOffset = 0;

		std::unordered_map<int, std::vector<vk::BufferCopy>> copyBatches;

		// 4. Process each NORMAL item
		// (Note: we iterate over normalBatches now, not the original allocPos)
		for (auto& tuple : normalBatches)
		{
			void* rawData = std::get<0>(tuple);
			uint32_t elementCount = std::get<1>(tuple);
			AllocationPosition* pos = std::get<2>(tuple);
			vk::DeviceSize byteSize = elementCount * DataSize;

			int targetBufferId = -1;
			uint32_t targetElementIndex = 0;

			for (int i = Buffers.size() - 1; i >= 0; i--)
			{
				auto& currentBuffer = *Buffers[i];
				if (currentBuffer.type != type) continue;

				if (currentBuffer.FreeSpace.empty())
				{
					if ((MaxCapacity - currentBuffer.offset) >= elementCount)
					{
						targetBufferId = i;
						targetElementIndex = currentBuffer.offset;
						currentBuffer.offset += elementCount;
						break;
					}
				}
				else
				{
					uint32_t location = (uint32_t)-1;
					uint32_t size = (uint32_t)-1;
					uint32_t currentCount = 0;
					uint8_t maxTests = 0;
					uint32_t targIndex = 0;

					for (auto& pair : currentBuffer.FreeSpace)
					{
						if (Preferences::Optimiser::UseDefragLimits && (currentCount > Preferences::Optimiser::maximumSearch || maxTests > Preferences::Optimiser::maximumTrial))
							break;

						if (pair.first >= elementCount && pair.first < size)
						{
							targIndex = currentCount;
							size = pair.first;
							location = pair.second;
							maxTests++;
						}
						currentCount++;
					}

					if (size == (uint32_t)-1 || location == (uint32_t)-1)
					{
						if ((MaxCapacity - currentBuffer.offset) >= elementCount)
						{
							targetBufferId = i;
							targetElementIndex = currentBuffer.offset;
							currentBuffer.offset += elementCount;
							break;
						}
					}
					else
					{
						targetBufferId = i;
						targetElementIndex = location;

						if (currentBuffer.FreeSpace[targIndex].first == elementCount) {
							currentBuffer.FreeSpace.erase(currentBuffer.FreeSpace.begin() + targIndex);
						}
						else {
							currentBuffer.FreeSpace[targIndex].first -= elementCount;
							currentBuffer.FreeSpace[targIndex].second += elementCount;
						}
						break;
					}
				}

			}

			if (targetBufferId == -1)
			{
				createBuffer(info, physicalDevice, type);
				targetBufferId = Buffers.size() - 1;
				targetElementIndex = 0;
				Buffers[targetBufferId]->offset += elementCount;
			}

			// Update AllocationPosition for the normal model
			pos->size = elementCount;
			pos->bufferId = targetBufferId;
			pos->startindex = targetElementIndex;

			memcpy(mappedData + currentStagingByteOffset, rawData, byteSize);

			vk::BufferCopy copyRegion;
			copyRegion.srcOffset = currentStagingByteOffset;
			copyRegion.dstOffset = targetElementIndex * DataSize;
			copyRegion.size = byteSize;

			copyBatches[targetBufferId].push_back(copyRegion);

			currentStagingByteOffset += byteSize;
		}

		stagingBufferMemory.unmapMemory();

		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = *info.CommandPool;
		allocInfo.level = vk::CommandBufferLevel::ePrimary;
		allocInfo.commandBufferCount = 1;

		vk::raii::CommandBuffer commandBulkCopyBuffer = std::move(info.device->allocateCommandBuffers(allocInfo).front());

		vk::CommandBufferBeginInfo cmdbufferBeginInfo{};
		cmdbufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandBulkCopyBuffer.begin(cmdbufferBeginInfo);

		for (const auto& [destId, regions] : copyBatches)
		{
			commandBulkCopyBuffer.copyBuffer(*stagingBuffer, *Buffers[destId]->buffer, regions);
		}

		commandBulkCopyBuffer.end();

		vk::SubmitInfo Submitinfo{};
		Submitinfo.commandBufferCount = 1;
		Submitinfo.pCommandBuffers = &*commandBulkCopyBuffer;

		info.queue->submit(Submitinfo, nullptr);
		info.queue->waitIdle();

		return RenderData::copyResult::Sucess;
	}
	void RenderData::MarkFreed(AllocationPosition& renderset)
	{
		if (renderset.bufferId >= Buffers.size())
		{
			Con::LogError("Buffer with id" + std::to_string(renderset.bufferId) + " does not exist!", HIGH_SEVERITY, TAG_ENGINE);
		}
		if (renderset.IsXL)
		{
			//Destroy this buffer
		}
		Buffers[renderset.bufferId]->FreeSpace.emplace_back(std::pair<uint32_t, uint32_t>(renderset.size, renderset.startindex));
	}
}