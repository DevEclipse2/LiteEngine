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
#include <iostream>
namespace lte
{
    class DeviceHandler;

    struct LtImage 
    {
        vk::raii::Image			image = nullptr;
        vk::raii::DeviceMemory	imageMemory = nullptr;
        vk::raii::Sampler		imageSampler = nullptr;
        vk::raii::ImageView		imageView = nullptr;
        uint32_t mipLevels = 0;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t channel = 0;
    };

    class ImageDelegate
    {
        public:
        
            [[nodiscard]] static void createImageView(uint32_t ltImage, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, vk::raii::Device* device);
            static void createImage(uint32_t image,uint32_t Width, uint32_t Height, uint32_t MipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice);
            ImageDelegate();
            ~ImageDelegate();
            static uint32_t requestImageCreation();
            static void requestImageDestruction(uint32_t index);
            //static void loadTextureFromDisk(std::string path, LtImage* ltImage, singleTimeCommandInfo info, vk::raii::PhysicalDevice* physDevice);
            static void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels, singleTimeCommandInfo info);
            static void generateMipmaps(uint32_t ltImage, vk::Format imageFormat, vk::raii::PhysicalDevice* physicalDevice, singleTimeCommandInfo info);
            static void createSwapchainImageViews(LtSwapChain* swap, vk::raii::Device* device);
            static LtImage* GetImagePtr(uint32_t index);

            static void createColorResources(LtSwapChain* swapChain, uint32_t ColorRes, vk::raii::Device* device, vk::raii::PhysicalDevice* physDev, vk::SampleCountFlagBits msaaSamples);
            static void createDepthResources(LtSwapChain* swapChain, uint32_t DepthRes, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice, vk::SampleCountFlagBits msaaSamples);
            static void transition_image_layout(
                vk::Image       image,
                vk::ImageLayout oldLayout,
                vk::ImageLayout newLayout,
                vk::AccessFlags2 srcAccessMask,
                vk::AccessFlags2 dstAccessMask,
                vk::PipelineStageFlags2 srcStageMask,
                vk::PipelineStageFlags2 dstStageMask,
                vk::ImageAspectFlags    image_aspect_flags,
                vk::raii::CommandBuffer* commandBuffer
            );
            //static void AllocatePointer
            static std::vector<LtImage> ImagePool; // this is not the correct method
        private:
            static std::vector<uint32_t> AvailableIndexes;

    };

    
}

