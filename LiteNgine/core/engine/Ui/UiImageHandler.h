#pragma once
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "../../vulkan/VulkanDevice.h"
#include "../../vulkan/Reworked/ImageDelegate.h"
namespace lte {

	class UiImageHandler
	{
		public:
			UiImageHandler();
			~UiImageHandler();
		private:
			void loadImg(std::string path, LtImage* img);
			VulkanDevice* pVulkanDevice;
			vk::PhysicalDeviceMemoryProperties properties;
	};

}