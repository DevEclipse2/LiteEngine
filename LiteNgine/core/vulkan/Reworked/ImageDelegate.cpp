#include "ImageDelegate.h"
namespace lte {

    struct LtImage {
        vk::raii::Image			image = nullptr;
        vk::raii::DeviceMemory	imageMemory = nullptr;
        vk::raii::Sampler		imageSampler = nullptr;
        vk::raii::ImageView		imageView = nullptr;
        uint32_t mipLevels = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channel = 0;
        const void createImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice)
        {
            width = Width;
            height = Height;
            mipLevels = MipLevels;
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D,
                imageInfo.format = format,
                imageInfo.extent = vk::Extent3D{ width, height, 1 },
                imageInfo.arrayLayers = 1,
                imageInfo.samples = vk::SampleCountFlagBits::e1,
                imageInfo.tiling = tiling,
                imageInfo.usage = usage,
                imageInfo.mipLevels = mipLevels;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.samples = numSamples;
            image = vk::raii::Image(*device, imageInfo);

            vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size,
                allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, properties, *physicalDevice);
            imageMemory = vk::raii::DeviceMemory(*device, allocInfo);
            image.bindMemory(imageMemory, 0);
        }

    }; struct LtImage {
        vk::raii::Image			image = nullptr;
        vk::raii::DeviceMemory	imageMemory = nullptr;
        vk::raii::Sampler		imageSampler = nullptr;
        vk::raii::ImageView		imageView = nullptr;
        uint32_t mipLevels = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channel = 0;
        const void createImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice)
        {
            width = Width;
            height = Height;
            mipLevels = MipLevels;
            vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D,
                imageInfo.format = format,
                imageInfo.extent = vk::Extent3D{ width, height, 1 },
                imageInfo.arrayLayers = 1,
                imageInfo.samples = vk::SampleCountFlagBits::e1,
                imageInfo.tiling = tiling,
                imageInfo.usage = usage,
                imageInfo.mipLevels = mipLevels;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.samples = numSamples;
            image = vk::raii::Image(*device, imageInfo);

            vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
            vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size,
                allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, properties, *physicalDevice);
            imageMemory = vk::raii::DeviceMemory(*device, allocInfo);
            image.bindMemory(imageMemory, 0);
        }

    };

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
    
    void ImageDelegate::requestImageDestruction(uint32_t Imageptr)
    {
        if (ImagePool.size() >= Imageptr + 1) {

            ImagePool[Imageptr].image = nullptr;
            ImagePool[Imageptr].imageMemory = nullptr;
            ImagePool[Imageptr].imageSampler = nullptr;
            ImagePool[Imageptr].imageView = nullptr;
            AvailableIndexes.emplace_back(Imageptr);
           
        }
        else 
        {
            //weird

        }
        
    }
    uint32_t ImageDelegate::requestImageCreation()
    {   
        if (AvailableIndexes.size() == 0) {
            ImagePool.emplace_back();
            return ImagePool.size() - 1;
        }
        else
        {
            uint32_t index = AvailableIndexes[0];
            AvailableIndexes.erase(AvailableIndexes.begin());
            return index;
        }
    }

    void ImageDelegate::createSwapchainImageViews( LtSwapChain* swap, vk::raii::Device* device)
    {
        assert(swap->imageViews.empty());

        vk::ImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.viewType = vk::ImageViewType::e2D,
            imageViewCreateInfo.format = swap->swapChainSurfaceFormat.format,
            imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

        for (auto& image : swap->swapChainImages)
        {
            imageViewCreateInfo.image = image;
            swap->imageViews.emplace_back(*device, imageViewCreateInfo);
        }
    }

    LtImage* ImageDelegate::GetImagePtr(uint32_t index) 
    {
        if (ImagePool.size() >= index + 1) 
        {
            return &ImagePool[index];
        }
        else
        {
            //weird

        }
    }

    void ImageDelegate::transition_image_layout(
        vk::Image       image,
        vk::ImageLayout oldLayout,
        vk::ImageLayout newLayout,
        vk::AccessFlags2 srcAccessMask,
        vk::AccessFlags2 dstAccessMask,
        vk::PipelineStageFlags2 srcStageMask,
        vk::PipelineStageFlags2 dstStageMask,
        vk::ImageAspectFlags    image_aspect_flags,
        vk::raii::CommandBuffer* commandBuffer
    )
    {
        if (image == nullptr) {
            std::cerr << "image provided is nullptr\n";
        }

        vk::ImageSubresourceRange range{};
        range.aspectMask = image_aspect_flags,
            range.baseMipLevel = 0,
            range.levelCount = 1,
            range.baseArrayLayer = 0,
            range.layerCount = 1;

        vk::ImageMemoryBarrier2 barrier = {};
        barrier.srcStageMask = srcStageMask,
            barrier.srcAccessMask = srcAccessMask,
            barrier.dstStageMask = dstStageMask,
            barrier.dstAccessMask = dstAccessMask,
            barrier.oldLayout = oldLayout,
            barrier.newLayout = newLayout,
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            barrier.image = image;
        barrier.subresourceRange = range;


        vk::DependencyInfo dependencyInfo = {};
        dependencyInfo.dependencyFlags = {},
            dependencyInfo.imageMemoryBarrierCount = 1,
            dependencyInfo.pImageMemoryBarriers = &barrier;

        commandBuffer->pipelineBarrier2(dependencyInfo);
    }

    void ImageDelegate::createDepthResources(LtSwapChain* swapChain,LtImage* DepthRes,vk::raii::Device* device ,vk::raii::PhysicalDevice* physicalDevice,vk::SampleCountFlagBits msaaSamples) {
        vk::Format depthFormat = PipelineDelegate::findDepthFormat(physicalDevice);
        DepthRes->createImage(swapChain->swapChainExtent.width, swapChain->swapChainExtent.height,0, msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, device, physicalDevice);
        //createImage(swapChainExtent.width, swapChainExtent.height, 1, vk::SampleCountFlagBits::e1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
        createImageView(DepthRes, depthFormat, vk::ImageAspectFlagBits::eDepth, 1,device);
    }
    

    void ImageDelegate::createColorResources(LtSwapChain* swapChain, LtImage* ColorRes, vk::raii::Device* device, vk::raii::PhysicalDevice* physDev, vk::SampleCountFlagBits msaaSamples) 
    {
        vk::Format colorFormat = swapChain->swapChainSurfaceFormat.format;
       
        ColorRes->createImage(swapChain->swapChainExtent.width, swapChain->swapChainExtent.height,0, msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, device, physDev);
        createImageView(ColorRes, colorFormat, vk::ImageAspectFlagBits::eColor, 1, device);
    }

    [[nodiscard]] void ImageDelegate::createImageView(LtImage* ltImage, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,vk::raii::Device* device) {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = ltImage->image,
            viewInfo.viewType = vk::ImageViewType::e2D,
            viewInfo.format = format,
            viewInfo.subresourceRange = { aspectFlags, 0, 1, 0, 1 };
        viewInfo.subresourceRange.levelCount = mipLevels;
        ltImage->imageView = vk::raii::ImageView(*device, viewInfo);
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