#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace lte
{
    static class ImageDelegate
    {

        struct LtImage {
            vk::raii::Image			image = nullptr;
            vk::raii::DeviceMemory	imageMemory = nullptr;
            vk::raii::Sampler		imageSampler = nullptr;
            vk::raii::ImageView		imageView = nullptr;

            const void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, vk::SampleCountFlagBits numSamples, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Device device, vk::raii::PhysicalDevice physicalDevice) {
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
                image = vk::raii::Image(device, imageInfo);

                vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
                vk::MemoryAllocateInfo allocInfo{};
                allocInfo.allocationSize = memRequirements.size,
                    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties , physicalDevice);
                imageMemory = vk::raii::DeviceMemory(device, allocInfo);
                image.bindMemory(imageMemory, 0);
            }
        };
        public:
            static uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice physicalDevice);


    };
}

