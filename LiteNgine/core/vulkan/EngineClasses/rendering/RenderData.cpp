#include "RenderData.h"
#include "../../Reworked/Buffers.h"
#include "../Preferences.h"
namespace lte {
	void RenderData::createVertexBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device)
	{
		//assert(vertexBuffer == nullptr);
		uint32_t ByteSize = MaxVertexBuffer * sizeof(Vertex);
		Buffer NewBuffer;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, NewBuffer.buffer, NewBuffer.Allocation, *info.device, device);
		Buffers.emplace_back(std::make_unique<Buffer>(NewBuffer));
	}
	RenderData::copyResult RenderData::copyVertexBufferContents(std::vector<Vertex> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice,RenderSet& renderset)
	{
		vk::BufferCreateInfo stagingInfo{};
		const auto& ByteSize = data.size() * sizeof(Vertex);
		stagingInfo.size = ByteSize,
		stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc,
		stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(*info.device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		renderset.vertexArraySize = data.size();
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *info.device, physicalDevice);
		void* dataStaging = stagingBufferMemory.mapMemory(0, ByteSize);
		memcpy(dataStaging, data.data(), ByteSize);
		stagingBufferMemory.unmapMemory();


		for (int i = Buffers.size() - 1; i >= 0; i--)//newest to oldest
		{
			auto& currentBuffer = *Buffers[i];
			if (currentBuffer.type != RenderData::BufferType::VertexBuffer)
			{
				//these are not the buffers you are looking for
				continue;
			}
			if (currentBuffer.FreeSpace.size() == 0)
			{
				if ((MaxVertexBuffer - currentBuffer.offset) > data.size())
				{
					//allocate memory at the back
					Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, currentBuffer.offset * sizeof(Vertex));
					renderset.bufferId = i;
					renderset.vertexArrayStartIndex = currentBuffer.offset;
					currentBuffer.offset += data.size();
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
					if (pair.first >= data.size() && pair.first < size)
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
					if ((MaxVertexBuffer - currentBuffer.offset) > data.size())
					{
						//allocate memory at the back
						Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, currentBuffer.offset * sizeof(Vertex));
						renderset.bufferId = i;
						renderset.vertexArrayStartIndex = currentBuffer.offset;
						currentBuffer.offset += data.size();
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
					Buffers::copyBufferIndexed(stagingBuffer, currentBuffer.buffer, ByteSize, info, 0, location * sizeof(Vertex));
					//here it reduces the size of the index 			
					renderset.bufferId = i;
					renderset.vertexArrayStartIndex = location;
					if (currentBuffer.FreeSpace[targIndex].first == data.size()) {
						currentBuffer.FreeSpace.erase(currentBuffer.FreeSpace.begin() + targIndex);
					}
					else {
						currentBuffer.FreeSpace[targIndex].first -= data.size();
						currentBuffer.FreeSpace[targIndex].second += data.size();
					}
					return RenderData::copyResult::Sucess;
				}
			}

		}
		//allocate new block 
		createVertexBuffer(info,physicalDevice);
		renderset.bufferId = Buffers.size() - 1;
		Buffers::copyBufferIndexed(stagingBuffer, Buffers[Buffers.size()-1]->buffer, ByteSize, info, 0, 0);
		renderset.vertexArrayStartIndex = 0;
		return RenderData::copyResult::Sucess;
	}
}