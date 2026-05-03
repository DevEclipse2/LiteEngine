#include "Buffers.h"
namespace lte {

	void Buffers::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory, vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice)
	{
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size,
			bufferInfo.usage = usage,
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		buffer = vk::raii::Buffer(device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size,
			allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,physDevice);
		bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}
	void Buffers::createVertexBuffer(uint32_t size , Vertex* vertex, vk::raii::Buffer* buffer, vk::raii::DeviceMemory* deviceMemory ,singleTimeCommandInfo info , vk::raii::PhysicalDevice& device)
	{
		//assert(vertexBuffer == nullptr);

		vk::BufferCreateInfo stagingInfo{};
		stagingInfo.size = size,
			stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc,
			stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(*info.device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;

		createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory , *info.device, device);
		//stagingBuffer.bindMemory(stagingBufferMemory, 0);
		void* dataStaging = stagingBufferMemory.mapMemory(0, size);

		memcpy(dataStaging, vertex, size);
		stagingBufferMemory.unmapMemory();
		createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, *buffer, *deviceMemory,*info.device,device);
		copyBuffer(stagingBuffer, *buffer, size, info);
	}

	void Buffers::createIndexBuffer(uint32_t size, uint32_t* vertex, vk::raii::Buffer* buffer, vk::raii::DeviceMemory* deviceMemory, singleTimeCommandInfo info, vk::raii::PhysicalDevice& device)
	{

		vk::raii::Buffer stagingBuffer = nullptr;
		vk::raii::DeviceMemory stagingBufferMemory = nullptr;
		createBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, *info.device,device);
		void* data = stagingBufferMemory.mapMemory(0, size);
		memcpy(data, vertex, size);
		//memcpy(data, indices.data(), (size_t)bufferSize);
		stagingBufferMemory.unmapMemory();
		createBuffer(size, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, *buffer, *deviceMemory,*info.device,device);
		copyBuffer(stagingBuffer, *buffer, size, info);

	}

	void Buffers::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size, singleTimeCommandInfo info) 
	{
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = *info.CommandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = 1;
		vk::raii::CommandBuffer commandCopyBuffer = std::move(info.device->allocateCommandBuffers(allocInfo).front());
		vk::CommandBufferBeginInfo cmdbuffer{};
		cmdbuffer.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandCopyBuffer.begin(cmdbuffer);
		commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
		commandCopyBuffer.end();
		vk::SubmitInfo Submitinfo{};
		Submitinfo.commandBufferCount = 1,
			Submitinfo.pCommandBuffers = &*commandCopyBuffer;
		info.queue->submit(Submitinfo, nullptr);

		info.queue->waitIdle();
	}
	void Buffers::createUniformBuffers(std::vector<LtMeshInfo>* meshes, uint8_t maxFIF, vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice)
	{

		for (auto& gameObject : *meshes) {
			gameObject.uniformBuffers.clear();
			gameObject.uniformBuffersMemory.clear();
			gameObject.uniformBuffersMapped.clear();

			// Create uniform buffers for each frame in flight
			for (size_t i = 0; i < maxFIF; i++) {
				vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
				vk::raii::Buffer buffer = nullptr;
				vk::raii::DeviceMemory bufferMem = nullptr;
				createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
					buffer, bufferMem, device,physDevice);
				gameObject.uniformBuffers.emplace_back(std::move(buffer));
				gameObject.uniformBuffersMemory.emplace_back(std::move(bufferMem));
				gameObject.uniformBuffersMapped.emplace_back(gameObject.uniformBuffersMemory[i].mapMemory(0, bufferSize));
			}
		}

	}
	void Buffers::copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height , singleTimeCommandInfo info) {
		std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = CommandBuffers::beginSingleTimeCommands(info.device,info.CommandPool);
		vk::BufferImageCopy                      region{};
		region.bufferOffset = 0,
			region.bufferRowLength = 0,
			region.bufferImageHeight = 0,
			region.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			region.imageOffset = vk::Offset3D{ 0, 0, 0 },
			region.imageExtent = vk::Extent3D{ width, height, 1 };
		commandBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, { region });
		CommandBuffers::endSingleTimeCommands(*commandBuffer,info.queue);
	}
}