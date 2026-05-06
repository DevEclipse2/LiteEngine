#include "FileLoader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_DISABLE_FAST_FLOAT
#include <tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
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
        //stbi_uc *pixels = stbi_load("textures/texture.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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

        /*transitionImageLayout(textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
        */
        ImageDelegate::transitionImageLayout(ImageIndex.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, ImageIndex.mipLevels,cmdInfo);
        Buffers::copyBufferToImage(stagingBuffer, ImageIndex.image, ImageIndex.width, ImageIndex.height, cmdInfo);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmap
        ImageDelegate::generateMipmaps(ImageIndex, vk::Format::eR8G8B8A8Srgb,physicalDevice,cmdInfo);
        //ImageDelegate::transitionImageLayout(ImageIndex.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, ImageIndex.mipLevels, cmdInfo);

    }

    void FileLoader::TemporaryFileLoad(vk::raii::Device& device, vk::raii::PhysicalDevice& physDevice, singleTimeCommandInfo info) 
    {
        objectCount = 0;
        //this temporarily loads files until i can find a better way to do it
        std::vector<std::string> models{ "models/viking_room.obj" , "models/Arrow.obj" };
        std::vector<std::string> textures{ "textures/viking_room.png","textures/Arrow.png" };

        for (int i = 0; i < models.size(); i++) {
            objectCount++;
            vertexBuf.emplace_back();
            indexBuf.emplace_back();
            LtImage tmpImg{};
            createTextureImage(textures[i], tmpImg, device, physDevice, info);
            ImageDelegate::createImageView(tmpImg, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, tmpImg.mipLevels, device);
            uint32_t imgIndex = ImageDelegate::requestImageCreation(tmpImg);

            imageIndexes.emplace_back(imgIndex);
            
            loadModel(&vertexBuf[i], &indexBuf[i], models[i]);
            //loadModel(&vertexBuf[i], &indexBuf[i], models[i]);
            
            ////multiple textures
            //prepareModels();
            //createTextureImage(i, textures[i], &imagesArr[i], &imageMem[i]);
            //createTextureImageView(&imagesArr[i]);
            //loadModel(i, models[i]);

        }
        
        VertexesSize = 0;
        IndicesSize = 0;
        VertexSizes.clear();
        IndiceSizes.clear();
        for (const auto& objF : vertexBuf) {
            VertexesSize += objF.size();
            VertexSizes.emplace_back(objF.size());
        }
        for (const auto& indice : indexBuf) {
            IndicesSize += indice.size();
            IndiceSizes.emplace_back(indice.size());
        }
        Vertex* newVertexes = new Vertex[VertexesSize];
        delete[] VertexArray;
        VertexArray = newVertexes;
        uint32_t* newIndices = new uint32_t[IndicesSize];
        delete[] IndicesArray;
        IndicesArray = newIndices;

        uint32_t Vindexes =0;
        uint32_t Iindexes =0;
        for (uint32_t i = 0; i < vertexBuf.size(); i++)
        {
            if (vertexBuf[i].size() == 0 ) continue;
            //starts at 0, length 12
            //renders from zero to 11
            //next one starts rendering from 12
            RenderSet rs{Vindexes,static_cast<uint32_t>(vertexBuf[i].size()),Iindexes,static_cast<uint32_t>(indexBuf[i].size()),imageIndexes[i]};
            renderSets.emplace_back(rs);
            //so we was reading garbage this whole time
            memcpy(VertexArray  + Vindexes, vertexBuf[i].data(), sizeof(Vertex) * vertexBuf[i].size());
            memcpy(IndicesArray + Iindexes, indexBuf[i].data(), sizeof(uint32_t) * indexBuf[i].size());
            Vindexes += static_cast<uint32_t>(vertexBuf[i].size());
            Iindexes += static_cast<uint32_t>(indexBuf[i].size());
  
        }

        imageIndexes.clear();
        vertexBuf.clear();
        indexBuf.clear();
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
}