#include "../VulkanDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace lte {
	void VulkanDevice::createTextureImage() {
        int texWidth, texHeight, texChannels;
        stbi_uc *pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        vk::raii::Buffer stagingBuffer({});
        vk::raii::DeviceMemory stagingBufferMemory({});
        createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

        transitionImageLayout(textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

    }
    void VulkanDevice::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory) {
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D, 
            imageInfo.format = format,
            imageInfo.extent = vk::Extent3D{width, height, 1},
            imageInfo.mipLevels = 1, 
            imageInfo.arrayLayers = 1,
            imageInfo.samples = vk::SampleCountFlagBits::e1, 
            imageInfo.tiling = tiling,
            imageInfo.usage = usage, 
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
        image = vk::raii::Image(device, imageInfo);

        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size,
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        imageMemory = vk::raii::DeviceMemory(device, allocInfo);
        image.bindMemory(imageMemory, 0);
    }


    void VulkanDevice::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
        auto commandBuffer = beginSingleTimeCommands();

        vk::ImageMemoryBarrier barrier{};
            barrier.oldLayout = oldLayout,
            barrier.newLayout = newLayout,
            barrier.image = image,
            barrier.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

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
        endSingleTimeCommands(*commandBuffer);
    }

    void VulkanDevice::copyBufferToImage(const vk::raii::Buffer& buffer, vk::raii::Image& image, uint32_t width, uint32_t height) {
        std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = beginSingleTimeCommands();
        vk::BufferImageCopy                      region{};
            region.bufferOffset = 0,
            region.bufferRowLength = 0,
            region.bufferImageHeight = 0,
            region.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
            region.imageOffset = vk::Offset3D{ 0, 0, 0 },
            region.imageExtent = vk::Extent3D{ width, height, 1 };
        commandBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, { region });
        endSingleTimeCommands(*commandBuffer);
    }

    void VulkanDevice::endSingleTimeCommands(vk::raii::CommandBuffer& commandBuffer) {
        commandBuffer.end();

        vk::SubmitInfo submitInfo{};
            submitInfo.commandBufferCount = 1, 
            submitInfo.pCommandBuffers = &*commandBuffer;
        queue.submit(submitInfo, nullptr);
        queue.waitIdle();
    }
    std::unique_ptr<vk::raii::CommandBuffer> VulkanDevice::beginSingleTimeCommands()
    {
        vk::CommandBufferAllocateInfo            allocInfo{};
            allocInfo.commandPool = commandPool, 
            allocInfo.level = vk::CommandBufferLevel::ePrimary, 
            allocInfo.commandBufferCount = 1;
        std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = std::make_unique<vk::raii::CommandBuffer>(std::move(vk::raii::CommandBuffers(device, allocInfo).front()));

        vk::CommandBufferBeginInfo beginInfo{};
            beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
        commandBuffer->begin(beginInfo);
        return commandBuffer;
    }



    void VulkanDevice::createTextureImageView() {
        textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb);

    }

    vk::raii::ImageView VulkanDevice::createImageView(vk::raii::Image& image, vk::Format format) {
        vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image,
            viewInfo.viewType = vk::ImageViewType::e2D,
            viewInfo.format = format, 
            viewInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
        return vk::raii::ImageView(device, viewInfo);
    }

    void VulkanDevice::createTextureSampler() {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter               = vk::Filter::eLinear, 
            samplerInfo.minFilter               = vk::Filter::eLinear, 
            samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear,
            samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat, 
            samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
            samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = vk::False;
            samplerInfo.compareEnable           = vk::False;
            samplerInfo.compareOp               = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
            samplerInfo.mipLodBias              = 0.0f;
            samplerInfo.minLod                  = 0.0f;
            samplerInfo.maxLod                  = 0.0f;
            textureSampler = vk::raii::Sampler(device, samplerInfo);
    }

}