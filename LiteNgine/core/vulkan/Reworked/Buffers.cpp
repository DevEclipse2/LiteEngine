#include "Buffers.h"
namespace lte {

	void Buffers::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory, vk::raii::Device* device)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size,
			bufferInfo.usage = usage,
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		buffer = vk::raii::Buffer(*device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size,
			allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		bufferMemory = vk::raii::DeviceMemory(*device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}
	void Buffers::copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height , vk::raii::Device* device, vk::CommandPool* pool , vk::raii::Queue* queue) {
		std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = CommandBuffers::beginSingleTimeCommands(device,pool);
		vk::BufferImageCopy                      region{};
		region.bufferOffset = 0,
			region.bufferRowLength = 0,
			region.bufferImageHeight = 0,
			region.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			region.imageOffset = vk::Offset3D{ 0, 0, 0 },
			region.imageExtent = vk::Extent3D{ width, height, 1 };
		commandBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, { region });
		CommandBuffers::endSingleTimeCommands(*commandBuffer,queue);
	}
}