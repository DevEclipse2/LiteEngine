#include "ImageDelegate.h"
namespace lte {
	
	void ImageDelegate::loadTextureFromDisk(std::string path, LtImage* ltImage , vk::raii::Device* device , vk::raii::PhysicalDevice* physDevice, vk::CommandPool* pool , vk::raii::Queue* queue) 
    {

        int width;
        int height;
        int channel;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        //stbi_uc *pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = width * height * 4;
        ltImage->mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        vk::raii::Buffer stagingBuffer = nullptr;
        vk::raii::DeviceMemory stagingBufferMemory = nullptr;
        Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, device);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);

        ltImage->createImage(width, height, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,device,physDevice);
        ltImage->width = width;
        ltImage->height = height;
        ltImage->channel = channel;
        
        transitionImageLayout(ltImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, ltImage->mipLevels , device , pool , queue);
        Buffers::copyBufferToImage(stagingBuffer, ltImage->image,ltImage->width, ltImage->height,device,pool,queue);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        generateMipmaps(*image, vk::Format::eR8G8B8A8Srgb, width, height, ltImage->mipLevels);

	}

    void ImageDelegate::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels , vk::raii::Device* device , vk::CommandPool* pool , vk::raii::Queue* queue) {
        auto commandBuffer = CommandBuffers::beginSingleTimeCommands(device, pool);

        vk::ImageMemoryBarrier barrier{};
        barrier.oldLayout = oldLayout,
            barrier.newLayout = newLayout,
            barrier.image = image,
            barrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        barrier.subresourceRange.levelCount = mipLevels;
        vk::PipelineStageFlags sourceStage;
        vk::PipelineStageFlags destinationStage;

        if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
            destinationStage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            sourceStage = vk::PipelineStageFlagBits::eTransfer;
            destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }
        commandBuffer->pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
        CommandBuffers::endSingleTimeCommands(*commandBuffer,queue);
    }
}