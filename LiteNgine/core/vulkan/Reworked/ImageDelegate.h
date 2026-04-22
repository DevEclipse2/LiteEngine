#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "DeviceHandler.h"
#include "CommandBuffers.h"
#include <stb_image.h>
#include "Buffers.h"
namespace lte
{
    static class ImageDelegate
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
            const void createImage(uint32_t width, uint32_t height, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice) 
            {
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
                    allocInfo.memoryTypeIndex = DeviceHandler::findMemoryType(memRequirements.memoryTypeBits, properties , *physicalDevice);
                imageMemory = vk::raii::DeviceMemory(*device, allocInfo);
                image.bindMemory(imageMemory, 0);
            }
        };
        public:
            void loadTextureFromDisk(std::string path, LtImage* ltImage, vk::raii::Device* device, vk::raii::PhysicalDevice* physDevice, vk::CommandPool* pool, vk::raii::Queue* queue);
            void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
            void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels, vk::raii::Device* device, vk::CommandPool* pool, vk::raii::Queue* queue);

    };
}

