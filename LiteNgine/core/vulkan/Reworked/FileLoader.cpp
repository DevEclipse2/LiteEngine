#include "FileLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "../EngineClasses/Lt_Console.h"
namespace lte {
    Vertex* FileLoader::VertexArray = nullptr;
    uint32_t FileLoader::VertexesSize = 0;
    uint32_t* FileLoader::IndicesArray = nullptr;
    uint32_t FileLoader::IndicesSize = 0;
    uint32_t FileLoader::objectCount = 2;
    std::vector<uint32_t> FileLoader::VertexSizes = {};
    std::vector<uint32_t> FileLoader::IndiceSizes = {};
    std::vector<RenderSet> FileLoader::renderSets = {};
    std::vector<std::vector<Vertex>> FileLoader::vertexBuf = {{}};
    std::vector<std::vector<uint32_t>> FileLoader::indexBuf = {{}};
    std::vector<uint32_t> FileLoader::imageIndexes = {};

    FileLoader::FileLoader()
    {
    }
    void FileLoader::createTextureImage(std::string path, LtImage& ImageIndex, vk::raii::Device& device , vk::raii::PhysicalDevice& physicalDevice,singleTimeCommandInfo cmdInfo)
    {

        int width, height, channel = 0;
        uint32_t mipLevels = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);

        //add handling to inject alpha into all

        vk::DeviceSize imageSize = width * height * 4;
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        vk::raii::Buffer stagingBuffer = nullptr;
        vk::raii::DeviceMemory stagingBufferMemory = nullptr;
        Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory,device,physicalDevice);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();

        stbi_image_free(pixels);
        
        ImageDelegate::createImage(ImageIndex,width, height,mipLevels,vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal,device,physicalDevice);
        ImageDelegate::createImageView(ImageIndex, vk::Format::eR8G8B8A8Srgb,vk::ImageAspectFlagBits::eColor,mipLevels,device);
        ImageDelegate::createSampler(ImageIndex,device);
        /*transitionImageLayout(textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        */
        ImageDelegate::transitionImageLayout(ImageIndex.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, ImageIndex.mipLevels,cmdInfo);
        Buffers::copyBufferToImage(stagingBuffer, ImageIndex.image, ImageIndex.width, ImageIndex.height, cmdInfo);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        ImageDelegate::generateMipmaps(ImageIndex, vk::Format::eR8G8B8A8Srgb,physicalDevice,cmdInfo);
        
    }

    void FileLoader::TemporaryFileLoad(vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice, singleTimeCommandInfo info) 
    {
    }

    void FileLoader::loadModel(std::vector<Vertex>* pVertices,std::vector<uint32_t>* pIndices,std::string path)
    {

        tinyobj::attrib_t                attrib;
        std::vector<tinyobj::shape_t>    shapes;
        std::vector<tinyobj::material_t> materials;
        std::string                      warn, err;
        /*bool LoadObj(attrib_t * attrib, std::vector<shape_t> *shapes, std::vector<material_t> *materials, std::string * err, const char* filename, const char* mtl_basedir = NULL,bool triangulate = true);*/

        std::cout << "loading!" << path.c_str() << "\n";
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            std::cout << (warn + err) << "\n";
        }
        std::cout << "loading complete! \n";


        std::unordered_map<Vertex, uint32_t> uniqueVertices;

        for (const auto& shape : shapes)
        {
            for (const auto& index : shape.mesh.indices)
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2] };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1] }; // throws index out of range exception

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (!uniqueVertices.contains(vertex))
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(pVertices->size());
                    pVertices->emplace_back(vertex);
                }

                pIndices->emplace_back(uniqueVertices[vertex]);
            }
        }
        std::cout << "indexing complete! \n";
    }
    bool FileLoader::ImGUIImg(std::string path, GUI_Image* img, vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice,singleTimeCommandInfo cmdInfo)
    {
        int width, height, channel = 0;
        uint32_t mipLevels = 0;
        stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channel, STBI_rgb_alpha);
        vk::DeviceSize imageSize = width * height * 4;
        mipLevels = 1;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
            return false;
        }
        

        ImageDelegate::createImage(img->image, width, height, mipLevels, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, device, physicalDevice);
        ImageDelegate::createImageView(img->image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, 1, device);
        ImageDelegate::createSampler(img->image, device);
        // Create Image View Descriptor Set 
        // (note: before 1.92.8 this also took a Sampler. See Wiki history)
        img->DS = ImGui_ImplVulkan_AddTexture(*img->image.imageSampler,*img->image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        
        vk::raii::Buffer stagingBuffer = nullptr;
        vk::raii::DeviceMemory stagingBufferMemory = nullptr;
        Buffers::createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory, device, physicalDevice);

        void* data = stagingBufferMemory.mapMemory(0, imageSize);
        memcpy(data, pixels, imageSize);
        stagingBufferMemory.unmapMemory();
        stbi_image_free(pixels);
        ImageDelegate::transitionImageLayout(img->image.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels, cmdInfo);
        Buffers::copyBufferToImage(stagingBuffer, img->image.image, width, height, cmdInfo);
        
        
        return true;
    }
    
}