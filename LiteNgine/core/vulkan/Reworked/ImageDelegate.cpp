#include "ImageDelegate.h"
namespace lte {
    ImageDelegate::ImageDelegate()
    {
        std::vector<LtImage> pool = {};
        ImagePool = pool;
    }

    ImageDelegate::~ImageDelegate()
    {
        //does not clear pointers
        ImagePool.clear();
    }
    
    void ImageDelegate::requestImageDestruction(LtImage* Imageptr)
    {
        if (ImagePool.size() > 0) {
            if (Imageptr >= ImagePool.data() && Imageptr < (ImagePool.data() + ImagePool.size())) {
                Imageptr->image = nullptr;
                Imageptr->imageMemory = nullptr;
                Imageptr->imageSampler = nullptr;
                Imageptr->imageView = nullptr;
            }
            else {
                //not part of the imagepool
            }
        }
        
    }
    LtImage* ImageDelegate::requestImageCreation()
    {   
        ImagePool.emplace_back();
        return &ImagePool[ImagePool.size() - 1];
    }
    void ImageDelegate::loadTextureFromDisk(std::string path, LtImage* ltImage , singleTimeCommandInfo info , vk::raii::PhysicalDevice* physDevice)
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
        Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, info.device);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);

        ltImage->createImage(width, height, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,device,physDevice);
        ltImage->width = width;
        ltImage->height = height;
        ltImage->channel = channel;
        
        transitionImageLayout(ltImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, ltImage->mipLevels ,info);
        Buffers::copyBufferToImage(stagingBuffer, ltImage->image,ltImage->width, ltImage->height,info);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        generateMipmaps(*image, vk::Format::eR8G8B8A8Srgb,physDevice,info);

	}

    void ImageDelegate::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels ,singleTimeCommandInfo info) 
    {
        auto commandBuffer = CommandBuffers::beginSingleTimeCommands(info.device, info.CommandPool);

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
        CommandBuffers::endSingleTimeCommands(*commandBuffer,info.queue);
    }

    void ImageDelegate::generateMipmaps(LtImage* ltImage,vk::Format imageFormat, vk::raii::PhysicalDevice* physicalDevice,singleTimeCommandInfo info) 
    {

        vk::FormatProperties formatProperties = physicalDevice->getFormatProperties(imageFormat);

        if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
        {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        std::unique_ptr<vk::raii::CommandBuffer> commandBuffer = CommandBuffers::beginSingleTimeCommands(info.device,info.CommandPool);


        vk::ImageMemoryBarrier barrier = {};
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead,
            barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal,
            barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal,
            barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
            barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
            barrier.image = ltImage->image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = ltImage->width;
        int32_t mipHeight = ltImage->height;

        for (uint32_t i = 1; i < ltImage->mipLevels; i++) {
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
                blit.dstOffsets = dstOffsets;
            blit.srcSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i - 1, 0, 1);
            blit.dstSubresource = vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1);
            commandBuffer->blitImage(ltImage->image, vk::ImageLayout::eTransferSrcOptimal, ltImage->image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);
            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        CommandBuffers::endSingleTimeCommands(*commandBuffer, info.queue);
    }
}