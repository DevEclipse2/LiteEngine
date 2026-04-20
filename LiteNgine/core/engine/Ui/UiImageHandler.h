#pragma once
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
#include "../../vulkan/VulkanDevice.h"
namespace lte {

	class UiImageHandler
	{
		struct GuiImage {
			vk::DescriptorSet descSet;
			uint16_t width = 0;
			uint16_t height = 0;
			uint16_t channels = 0;

			vk::ImageView imgView;
			vk::Image img;
			vk::DeviceMemory imageMem;
			vk::Sampler sampler;
			vk::Buffer uploadBuffer;
			vk::DeviceMemory uploadBufferMemory;

			GuiImage() { memset(this, 0, sizeof(this)); }
		};
		public:
			UiImageHandler();
			~UiImageHandler();
		private:
			void loadImg(std::string path, GuiImage* img);
			VulkanDevice* pVulkanDevice;
			vk::PhysicalDeviceMemoryProperties properties;
	};

}