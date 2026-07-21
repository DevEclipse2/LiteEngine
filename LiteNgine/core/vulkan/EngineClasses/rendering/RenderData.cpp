#include "RenderData.h"
#include "../../Reworked/Buffers.h"
namespace lte {
	void RenderData::createVertexBuffer(singleTimeCommandInfo info, vk::raii::PhysicalDevice& device)
	{
		//assert(vertexBuffer == nullptr);
		uint32_t ByteSize = MaxVertexBuffer * sizeof(Vertex);
		
		Buffer NewBuffer;
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, NewBuffer.buffer, NewBuffer.bufferMem, *info.device, device);
		Buffers.emplace_back(std::make_unique<Buffer>(NewBuffer));
	}
	RenderData::copyResult RenderData::copyVertexBufferContents(std::vector<Vertex> data, singleTimeCommandInfo info, vk::raii::PhysicalDevice& physicalDevice)
	{
		vk::BufferCreateInfo stagingInfo{};
		const auto& ByteSize = data.size() * sizeof(Vertex);
		stagingInfo.size = ByteSize,
		stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc,
		stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(*info.device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		int ValidIndex = -1;
		for (int i = Buffers.size() - 1; i >= 0; i--)
		{
			if (Buffers[i]->type != RenderData::BufferType::VertexBuffer)
			{
				continue;
			}
			if ((MaxVertexBuffer - Buffers[i]->offset) > data.size())
			{

			}
		}
		if (ValidIndex == -1)
		{
			//allocate new block 
		}
		Buffers::createBuffer(ByteSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *info.device, physicalDevice);
		//stagingBuffer.bindMemory(stagingBufferMemory, 0);
		void* dataStaging = stagingBufferMemory.mapMemory(0, ByteSize);

		memcpy(dataStaging, vertex, ByteSize);
		stagingBufferMemory.unmapMemory();
		Buffers::copyBuffer(stagingBuffer, *buffer, ByteSize, info);
		return RenderData::copyResult::Sucess;
	}
}