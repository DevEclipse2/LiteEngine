#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "DeviceHandler.h"
#include "CommandBuffers.h"
#include "SwapchainHandler.h"
#include "PipelineDelegate.h"
#include <stb_image.h>
#include "Buffers.h"
#include <stdlib.h>
namespace lte
{
    struct LtImage {
        vk::raii::Image			image = nullptr;
        vk::raii::DeviceMemory	imageMemory = nullptr;
        vk::raii::Sampler		imageSampler = nullptr;
        vk::raii::ImageView		imageView = nullptr;
        uint32_t mipLevels = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channel = 0;
        const void createImage(uint32_t Width, uint32_t Height, uint32_t MipLevels,vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice)
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
    static class ImageDelegate
    {
        public:
        
       
            ImageDelegate();
            ~ImageDelegate();
            static LtImage* requestImageCreation();
            static void requestImageDestruction(LtImage*);
            static void loadTextureFromDisk(std::string path, LtImage* ltImage, singleTimeCommandInfo info, vk::raii::PhysicalDevice* physDevice);
            static void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels, singleTimeCommandInfo info);
            static void generateMipmaps(LtImage* ltImage, vk::Format imageFormat, vk::raii::PhysicalDevice* physicalDevice, singleTimeCommandInfo info);
            static void createSwapchainImageViews(LtSwapChain* swap, vk::raii::Device* device);
            static void createColorResources(LtSwapChain* swapChain, LtImage* ColorRes, vk::raii::Device* device, vk::raii::PhysicalDevice* physDev, vk::SampleCountFlagBits msaaSamples);
            [[nodiscard]] static void createImageView(LtImage* ltImage, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::raii::Device* device);
            static void createDepthResources(LtSwapChain* swapChain, LtImage* DepthRes, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice, vk::SampleCountFlagBits msaaSamples);

            static void AllocatePointer

        private:
            static std::vector<LtImage> ImagePool; // this is not the correct method


    };
}

