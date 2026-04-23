#include "FileLoader.h"
namespace lte {

    void FileLoader::createTextureImage(std::string path, LtImage* Image, vk::raii::Device* device , vk::raii::PhysicalDevice* physicalDevice,singleTimeCommandInfo cmdInfo)
    {

        int width, height, channel = 0;
        uint32_t mipLevels = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        //stbi_uc *pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vk::DeviceSize imageSize = width * height * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        vk::raii::Buffer stagingBuffer = nullptr;
        vk::raii::DeviceMemory stagingBufferMemory = nullptr;
        Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory,device);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);
        Image->createImage(width, height,mipLevels,vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,device,physicalDevice);

        /*transitionImageLayout(textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        */
        ImageDelegate::transitionImageLayout(Image->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, Image->mipLevels,cmdInfo);
        Buffers::copyBufferToImage(stagingBuffer, Image->image, Image->width, Image->height,cmdInfo);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        ImageDelegate::generateMipmaps(Image, vk::Format::eR8G8B8A8Srgb,physicalDevice,cmdInfo);

    }

    void FileLoader::TemporaryFileLoad() {
        objectCount = 0;
        //this temporarily loads files until i can find a better way to do it
        std::vector<std::string> models{ "models/viking_room.obj" , "models/Arrow.obj" };
        std::vector<std::string> textures{ "textures/viking_room.png","textures/Arrow.png" };

        for (int i = 0; i < models.size(); i++) {
            objectCount++;
            LtImage* Iptr = ImageDelegate::requestImageCreation();
            createTextureImage(textures[i],)
            ////multiple textures
            //prepareModels();
            //createTextureImage(i, textures[i], &imagesArr[i], &imageMem[i]);
            //createTextureImageView(&imagesArr[i]);
            //loadModel(i, models[i]);

        }

    }
}