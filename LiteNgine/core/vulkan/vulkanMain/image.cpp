#include "../VulkanDevice.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
namespace lte {
    void VulkanDevice::createTextureImage(uint32_t index, std::string path, vk::raii::Image* image, vk::raii::DeviceMemory* mem, int* width, int* height, int* channel)
    {
        stbi_uc* pixels = stbi_load(path.c_str(), width, height, channel, STBI_rgb_alpha);
        //stbi_uc *pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = *width * *height * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(*width, *height)))) + 1;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        vk::raii::Buffer stagingBuffer = nullptr;
        vk::raii::DeviceMemory stagingBufferMemory= nullptr;
        createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);

        createImage(*width, *height, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, *image, *mem);

        /*transitionImageLayout(textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        */

        transitionImageLayout(*image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer,*image, static_cast<uint32_t>(*width), static_cast<uint32_t>(*height));
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        generateMipmaps(*image, vk::Format::eR8G8B8A8Srgb, *width, *height, mipLevels);

    }
    void VulkanDevice::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Image& image, vk::raii::DeviceMemory& imageMemory){
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D, 
            imageInfo.format = format,
            imageInfo.extent = vk::Extent3D{width, height, 1},
            imageInfo.arrayLayers = 1,
            imageInfo.samples = vk::SampleCountFlagBits::e1, 
            imageInfo.tiling = tiling,
            imageInfo.usage = usage, 
            imageInfo.mipLevels = mipLevels;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.samples = numSamples;
        image = vk::raii::Image(device, imageInfo);

        vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size,
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        imageMemory = vk::raii::DeviceMemory(device, allocInfo);
        image.bindMemory(imageMemory, 0);
    }


    void VulkanDevice::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) {
        auto commandBuffer = beginSingleTimeCommands();

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



    void VulkanDevice::createTextureImageView(vk::raii::Image* image) {
        imageViewArr.emplace_back(createImageView(*image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, mipLevels));

    }

    [[nodiscard]] vk::raii::ImageView VulkanDevice::createImageView(const vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels) {
        vk::ImageViewCreateInfo viewInfo{};
            viewInfo.image = image,
            viewInfo.viewType = vk::ImageViewType::e2D,
            viewInfo.format = format, 
            viewInfo.subresourceRange = { aspectFlags, 0, 1, 0, 1 };
            viewInfo.subresourceRange.levelCount = mipLevels;
        return vk::raii::ImageView(device, viewInfo);
    }

    void VulkanDevice::createTextureSampler(vk::raii::Sampler* sampler) {
        vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
        vk::SamplerCreateInfo samplerInfo{};
            samplerInfo.magFilter               = vk::Filter::eLinear, 
            samplerInfo.minFilter               = vk::Filter::eLinear, 
            samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear,
            samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat, 
            samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
            samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat,
            samplerInfo.mipLodBias = 0.0f,
            samplerInfo.anisotropyEnable = vk::True,
            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
            samplerInfo.compareEnable = vk::False,
            samplerInfo.compareOp = vk::CompareOp::eAlways,
            samplerInfo.minLod = 0.0f,
            samplerInfo.maxLod = vk::LodClampNone,
            /*samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
            samplerInfo.unnormalizedCoordinates = vk::False;
            samplerInfo.compareEnable           = vk::False;
            samplerInfo.compareOp               = vk::CompareOp::eAlways;
            samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
            samplerInfo.mipLodBias              = 0.0f;
            samplerInfo.minLod                  = 0.0f;
            samplerInfo.maxLod                  = 0.0f;*/
        *sampler = vk::raii::Sampler(device, samplerInfo);
    }

    void VulkanDevice::generateMipmaps(vk::raii::Image& image, vk::Format imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {
        
        vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = beginSingleTimeCommands();

        
        vk::ImageMemoryBarrier barrier = {};
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead,
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal,
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal,
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            barrier.image = image;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, barrier);
            vk::ArrayWrapper1D<vk::Offset3D, 2> offsets, dstOffsets;
            offsets[0] = vk::Offset3D(0, 0, 0);
            offsets[1] = vk::Offset3D(mipWidth, mipHeight, 1);
            dstOffsets[0] = vk::Offset3D(0, 0, 0);
            dstOffsets[1] = vk::Offset3D(mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1);
            vk::ImageBlit blit = {};
                //blit.srcSubresource = {},
                blit.srcOffsets = offsets,
                //blit.dstSubresource = {},
                blit.dstOffsets = dstOffsets ;
            blit.srcSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
            blit.dstSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1);
            commandBuffer->blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);
            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        endSingleTimeCommands(*commandBuffer);
    }

    void VulkanDevice::createColorResources() {
        vk::Format colorFormat = swapChainSurfaceFormat.format;

        createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, colorImage, colorImageMemory);
        colorImageView = createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1);
    }
}