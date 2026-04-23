#pragma once
#include <vulkan/vulkan_raii.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "ImageDelegate.h"
#include "Buffers.h"
//#include <string>
namespace lte 
{
	class FileLoader
	{

		public:
			static void createTextureImage(std::string path, LtImage* Image, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice, singleTimeCommandInfo cmdInfo);
			void TemporaryFileLoad();
			static uint32_t objectCount;
			std::vector<LtImage*> textureImages;

	};

}