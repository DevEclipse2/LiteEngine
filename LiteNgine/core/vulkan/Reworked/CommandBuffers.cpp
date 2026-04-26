#include "CommandBuffers.h"
namespace lte {
   

    std::unique_ptr<vk::raii::CommandBuffer> CommandBuffers::beginSingleTimeCommands(vk::raii::Device* device, vk::raii::CommandPool* commandPool)
    {
        vk::CommandBufferAllocateInfo            allocInfo{};
        allocInfo.commandPool = *commandPool,
            allocInfo.level = vk::CommandBufferLevel::ePrimary,
            allocInfo.commandBufferCount = 1;
        std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(*device, allocInfo).front()));

        vk::CommandBufferBeginInfo beginInfo{};
        beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        commandBuffer->begin(beginInfo);
        return commandBuffer;
    }
    void CommandBuffers::endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer , vk::raii::Queue* queue) 
    {
        commandBuffer.end();

        vk::SubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1,
            submitInfo.pCommandBuffers = &*commandBuffer;
        queue->submit(submitInfo, nullptr);
        queue->waitIdle();
    }
    void CommandBuffers::createCommandPool(vk::raii::CommandPool* commandPool, vk::raii::Device* device , uint32_t queueIndex)
    {
        vk::CommandPoolCreateInfo poolInfo{};
        poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            poolInfo.queueFamilyIndex = queueIndex;
        *commandPool = vk::raii::CommandPool(*device, poolInfo);
    }
    void CommandBuffers::createCommandBuffer(std::vector<vk::raii::CommandBuffer>* commandBuffers, vk::raii::CommandPool* commandPool, vk::raii::Device* device, uint8_t maxFIF) 
    {

        commandBuffers->clear();
        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandPool = **commandPool,
            allocInfo.level = vk::CommandBufferLevel::ePrimary,
            allocInfo.commandBufferCount = maxFIF;
        *commandBuffers = vk::raii::CommandBuffers(*device, allocInfo);

        /*vk::CommandBufferAllocateInfo allocInfo{};
            allocInfo.commandPool = commandPool,
            allocInfo.level = vk::CommandBufferLevel::ePrimary,
            allocInfo.commandBufferCount = 1 ;
        commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());*/
    }
}