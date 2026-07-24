#include "ImageDelegate.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <thread>
namespace lte {

    std::vector<uint32_t> ImageDelegate::AvailableIndexes = {};
    std::vector<std::unique_ptr<lte::LtImage>> ImageDelegate::ImagePool = {};


    void ImageDelegate::createImage(LtImage& image,uint32_t Width, uint32_t Height, uint32_t MipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice)
    {
        image.width = Width;
        image.height = Height;
        image.mipLevels = MipLevels;
        vk::ImageCreateInfo imageInfo{};
            imageInfo.imageType = vk::ImageType::e2D,
            imageInfo.format = format,
            imageInfo.extent = vk::Extent3D{ image.width, image.height, 1 },
            imageInfo.arrayLayers = 1,
            imageInfo.samples = vk::SampleCountFlagBits::e1,
            imageInfo.tiling = tiling,
            imageInfo.usage = usage,
            imageInfo.mipLevels = image.mipLevels;
            imageInfo.sharingMode = vk::SharingMode::eExclusive;
            imageInfo.samples = numSamples;
           
        image.image = vk::raii::Image(device, imageInfo);

        vk::MemoryRequirements memRequirements = image.image.getMemoryRequirements();
        vk::MemoryAllocateInfo allocInfo{};
            allocInfo.allocationSize = memRequirements.size,
            allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, properties, physicalDevice);
        image.imageMemory = vk::raii::DeviceMemory(device, allocInfo);
        image.image.bindMemory(image.imageMemory, 0);
    }

    ImageDelegate::ImageDelegate()
    {
       
        ImagePool.reserve(12);
        /*std::vector<LtImage> pool = {};
        ImagePool = pool;*/
    }

    ImageDelegate::~ImageDelegate()
    {
        //does not clear pointers
        ImagePool.clear();
    }
    
    void ImageDelegate::requestDelayedImageDestruction(uint32_t imageIndex, float wait)
    {

    }

    void ImageDelegate::createSampler(LtImage& image, vk::raii::Device& device)
    {
        vk::SamplerCreateInfo samplerInfo{};
        samplerInfo.magFilter = vk::Filter::eLinear;                    
        samplerInfo.minFilter = vk::Filter::eLinear;                    
        samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;        // Smooth transitions between mip levels
        samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;  // Prevent texture wrapping
        samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;  // Clean edge handling
        samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;  // 3D consistency
        samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;   // White border for clamped areas
        image.imageSampler = vk::raii::Sampler(device, samplerInfo);                   // Create the GPU sampler object
    }

    void ImageDelegate::requestImageDestruction(uint32_t& imageIndex)
    {
        if (imageIndex < ImagePool.size()) {
            ImagePool[imageIndex]->image        = nullptr;
            ImagePool[imageIndex]->imageMemory  = nullptr;
            ImagePool[imageIndex]->imageSampler = nullptr;
            ImagePool[imageIndex]->imageView    = nullptr;
            AvailableIndexes.emplace_back(imageIndex);
            imageIndex = -1;
        }
    }
    uint32_t ImageDelegate::requestImageCreation(LtImage& image)
    {   
        if (AvailableIndexes.size() > 0)
        {
            ImagePool[AvailableIndexes[0]]= std::make_unique<LtImage>(std::move(image));
            uint32_t index = AvailableIndexes[0];
            AvailableIndexes.erase(AvailableIndexes.begin());
            return index;

        }
        else {
            ImagePool.emplace_back(std::make_unique<LtImage>(std::move(image)));
            //ImagePool.emplace_back(std::move(image)); // no matching overloaded function founded
            //ImagePool.emplace_back(std::move({})); // no matching overloaded function founded
            //ImagePool.emplace_back(std::move(image)); // no matching overloaded function founded
            return ImagePool.size() - 1;
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
            swap->imageViews.emplace_back(std::move(vk::raii::ImageView{ *device, imageViewCreateInfo }));
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
        vk::raii::CommandBuffer& commandBuffer
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

        commandBuffer.pipelineBarrier2(dependencyInfo);
    }

    void ImageDelegate::DumpImages(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice, VkCommandPool commandPool,
            VkQueue queue, VkImage image, uint32_t width, uint32_t height, const char* filename)
    {
        VkDeviceSize imageSize = width * height * 4;

        // 1. Create a temporary R8G8B8A8_UNORM image to blit into
        VkImage tempImage;
        VkDeviceMemory tempImageMemory;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { width, height, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Force standard RGBA output
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        vkCreateImage(*device, &imageInfo, nullptr, &tempImage);

        VkMemoryRequirements memReq;
        vkGetImageMemoryRequirements(*device, tempImage, &memReq);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal,physicalDevice);
        vkAllocateMemory(*device, &allocInfo, nullptr, &tempImageMemory);
        vkBindImageMemory(*device, tempImage, tempImageMemory, 0);

        // 2. Create host-visible buffer for final CPU read
        VkBuffer buffer;
        VkDeviceMemory bufferMemory;
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        vkCreateBuffer(*device, &bufferInfo, nullptr, &buffer);

        vkGetBufferMemoryRequirements(*device, buffer, &memReq);
        allocInfo.allocationSize = memReq.size;
        allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memReq.memoryTypeBits,vk::MemoryPropertyFlagBits::eHostVisible| vk::MemoryPropertyFlagBits::eHostCoherent,physicalDevice);
        vkAllocateMemory(*device, &allocInfo, nullptr, &bufferMemory);
        vkBindBufferMemory(*device, buffer, bufferMemory, 0);

        // 3. Begin single-use command buffer
        VkCommandBufferAllocateInfo allocCmdInfo{};
        allocCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocCmdInfo.commandPool = commandPool;
        allocCmdInfo.commandBufferCount = 1;
        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(*device, &allocCmdInfo, &cmd);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmd, &beginInfo);

        // 4. Transition layouts for Blit
        auto transitionLayout = [&](VkImage img, VkImageLayout oldLayout, VkImageLayout newLayout,
            VkAccessFlags srcAcc, VkAccessFlags dstAcc,
            VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
                VkImageMemoryBarrier barrier{};
                barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                barrier.oldLayout = oldLayout;
                barrier.newLayout = newLayout;
                barrier.image = img;
                barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                barrier.srcAccessMask = srcAcc;
                barrier.dstAccessMask = dstAcc;
                vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            };

        // Transition User Image -> TRANSFER_SRC
        transitionLayout(image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        // Transition Temp Image -> TRANSFER_DST
        transitionLayout(tempImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        // 5. Blit (This automatically converts the format!)
        VkImageBlit blit{};
        blit.srcOffsets[1] = { (int32_t)width, (int32_t)height, 1 };
        blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        blit.dstOffsets[1] = { (int32_t)width, (int32_t)height, 1 };
        blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        vkCmdBlitImage(cmd, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            tempImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

        // 6. Transition Temp Image -> TRANSFER_SRC
        transitionLayout(tempImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        // 7. Copy Temp Image to Host Buffer
        VkBufferImageCopy copyRegion{};
        copyRegion.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
        copyRegion.imageExtent = { width, height, 1 };
        vkCmdCopyImageToBuffer(cmd, tempImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copyRegion);

        // 8. Put User Image back to SHADER_READ_ONLY
        transitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        // 9. Execute and Wait
        vkEndCommandBuffer(cmd);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue); // Wait for the GPU to finish the blit (usually < 1ms)

        // 10. Copy data to CPU memory FAST
        void* data;
        vkMapMemory(*device, bufferMemory, 0, imageSize, 0, &data);

        // Copy the raw VRAM data into a standard vector
        std::vector<uint8_t> pixelData((uint8_t*)data, (uint8_t*)data + imageSize);

        vkUnmapMemory(*device, bufferMemory);

        // 11. Cleanup Vulkan resources IMMEDIATELY on the main thread (thread-safe)
        vkFreeCommandBuffers(*device, commandPool, 1, &cmd);
        vkDestroyBuffer(*device, buffer, nullptr);
        vkFreeMemory(*device, bufferMemory, nullptr);
        vkDestroyImage(*device, tempImage, nullptr);
        vkFreeMemory(*device, tempImageMemory, nullptr);

        // 12. Spin up a detached background thread to do the heavy PNG compression
        std::string safeFilename = filename;
        std::thread([pixels = std::move(pixelData), width, height, safeFilename]() {

            // This will take 100ms+, but the main application will keep running smoothly!
            stbi_write_png(safeFilename.c_str(), width, height, 4, pixels.data(), width * 4);

            }).detach();
    }
    void ImageDelegate::createDepthResources(LtSwapChain* swapChain,LtImage& DepthRes,vk::raii::Device& device ,vk::raii::PhysicalDevice& physicalDevice,vk::SampleCountFlagBits msaaSamples) 
    {
        vk::Format depthFormat = PipelineDelegate::findDepthFormat(physicalDevice);
        createImage(DepthRes,swapChain->swapChainExtent.width, swapChain->swapChainExtent.height,1, msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, device, physicalDevice);
        //createImage(swapChainExtent.width, swapChainExtent.height, 1, vk::SampleCountFlagBits::e1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
        createImageView(DepthRes, depthFormat, vk::ImageAspectFlagBits::eDepth, 1,device);
    }
    

    void ImageDelegate::createColorResources(LtSwapChain* swapChain, LtImage& ColorRes, vk::raii::Device& device, vk::raii::PhysicalDevice& physDev, vk::SampleCountFlagBits msaaSamples) 
    {
        vk::Format colorFormat = swapChain->swapChainSurfaceFormat.format;
       
        createImage(ColorRes,swapChain->swapChainExtent.width, swapChain->swapChainExtent.height,1, msaaSamples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, device, physDev);
        createImageView(ColorRes, colorFormat, vk::ImageAspectFlagBits::eColor, 1, device);
    }

    [[nodiscard]] void ImageDelegate::createImageView(LtImage& ltImage, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels,vk::raii::Device& device) {
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = *(ltImage.image),
            viewInfo.viewType = vk::ImageViewType::e2D,
            viewInfo.format = format,
            viewInfo.subresourceRange = { aspectFlags, 0, 1, 0, 1 };
        viewInfo.subresourceRange.levelCount = mipLevels;
        ltImage.imageView = vk::raii::ImageView(device, viewInfo);
    }

    

    void ImageDelegate::transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels ,singleTimeCommandInfo info) 
    {
        auto commandBuffer = CommandBuffers::beginSingleTimeCommands(info.device,info.CommandPool);

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
    void ImageDelegate::Terminate() 
    {
        ImagePool.clear();
    }

    void ImageDelegate::generateMipmaps(LtImage& ltImage,vk::Format imageFormat, vk::raii::PhysicalDevice& physicalDevice,singleTimeCommandInfo info) 
    {


        vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(imageFormat);

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
            barrier.image = ltImage.image;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = ltImage.width;
        int32_t mipHeight = ltImage.height;

        for (uint32_t i = 1; i < ltImage.mipLevels; i++) {
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
            commandBuffer->blitImage(ltImage.image, vk::ImageLayout::eTransferSrcOptimal,ltImage.image, vk::ImageLayout::eTransferDstOptimal, { blit }, vk::Filter::eLinear);
            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);
            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = ltImage.mipLevels - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, barrier);

        CommandBuffers::endSingleTimeCommands(*commandBuffer, info.queue);
    }
}