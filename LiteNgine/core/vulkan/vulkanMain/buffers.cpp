#include "../VulkanDevice.h"
namespace lte {
	void VulkanDevice::createUniformBuffers() {

		for (auto& gameObject : meshes) {
			gameObject.uniformBuffers.clear();
			gameObject.uniformBuffersMemory.clear();
			gameObject.uniformBuffersMapped.clear();

			// Create uniform buffers for each frame in flight
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
				vk::raii::Buffer buffer= nullptr;
				vk::raii::DeviceMemory bufferMem= nullptr;
				createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
					buffer, bufferMem);
				gameObject.uniformBuffers.emplace_back(std::move(buffer));
				gameObject.uniformBuffersMemory.emplace_back(std::move(bufferMem));
				gameObject.uniformBuffersMapped.emplace_back(gameObject.uniformBuffersMemory[i].mapMemory(0, bufferSize));
			}
		}

		/*uniformBuffers.clear();
		uniformBuffersMemory.clear();
		uniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer buffer({});
			vk::raii::DeviceMemory bufferMem({});
			createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
			uniformBuffers.emplace_back(std::move(buffer));
			uniformBuffersMemory.emplace_back(std::move(bufferMem));
			uniformBuffersMapped.emplace_back(uniformBuffersMemory[i].mapMemory(0, bufferSize));
		}*/
	}
	void VulkanDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
		vk::BufferCreateInfo bufferInfo{};
		bufferInfo.size = size,
			bufferInfo.usage = usage,
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		buffer = vk::raii::Buffer(device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{};
		allocInfo.allocationSize = memRequirements.size,
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}
	void VulkanDevice::updateUniformBuffer(uint32_t frame)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		fps = 1 / (time - prevtime);
		frameTime = (time - prevtime) * 1000;
		UniformBufferObject ubo{};

		glm::mat4 view = glm::lookAt(glm::vec3(2.0f, -6.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
			0.1f, 20.0f);

		ubo.proj[1][1] *= -1;
		// Update uniform buffers for each object
		for (auto& gameObject : meshes) {
			// Apply continuous rotation to the object
			const float rotationSpeed = 0.5f;                          // Rotation speed in radians per second
			gameObject.rotation.y += rotationSpeed * (time - prevtime);

			// Get the model matrix for this object
			glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 model = gameObject.getModelMatrix() * initialRotation;

			// Create and update the UBO
			UniformBufferObject ubo{};
			ubo.model = model,
				ubo.view = view,
				ubo.proj = proj;


			// Copy the UBO data to the mapped memory
			memcpy(gameObject.uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
		}
		prevtime = time;
	}


	void VulkanDevice::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = commandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = 1;
		vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());
		vk::CommandBufferBeginInfo cmdbuffer{};
		cmdbuffer.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandCopyBuffer.begin(cmdbuffer);
		commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
		commandCopyBuffer.end();
		vk::SubmitInfo info{};
		info.commandBufferCount = 1,
			info.pCommandBuffers = &*commandCopyBuffer;
		queue.submit(info, nullptr);

		queue.waitIdle();
	}
}