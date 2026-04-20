#include "UiImageHandler.h"
namespace lte {
	UiImageHandler::UiImageHandler()
	{

	}
	void UiImageHandler::loadImg(std::string path, GuiImage* img)
	{
		vk::raii::Image imageTemp = nullptr;
		vk::raii::DeviceMemory deviceMem = nullptr;
		vk::raii::ImageView imageView = nullptr;
		vk::raii::Sampler sampler = nullptr;
		VkBuffer uploadBuffer;
		VkDeviceMemory mem;
		int width;
		int height;
		int channel;
		size_t image_size = width * height * channel;
		pVulkanDevice->createTextureImage(0,path,&imageTemp,&deviceMem,&width,&height,&channel);
		imageView = pVulkanDevice->createImageView(imageTemp, vk::Format::eR8G8B8Unorm, vk::ImageAspectFlagBits::eColor, 1);
		pVulkanDevice->createTextureSampler(&sampler);
		vk::DescriptorSet descSet = ImGui_ImplVulkan_AddTexture(*sampler, *imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkResult err;
		VkBufferCreateInfo buffer_info = {};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = image_size;
		buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vkCreateBuffer(**pVulkanDevice->getDevice(), &buffer_info, NULL, &uploadBuffer);
		VkMemoryRequirements req;
		vkGetBufferMemoryRequirements(**pVulkanDevice->getDevice(),uploadBuffer, &req);
		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = req.size;
		alloc_info.memoryTypeIndex = pVulkanDevice->findMemoryType(req.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible);
		err = vkAllocateMemory(**pVulkanDevice->getDevice(), &alloc_info, NULL, &mem);
		err = vkBindBufferMemory(**pVulkanDevice->getDevice(), uploadBuffer, mem, 0);
		
		GuiImage img = { descSet,static_cast<uint16_t>(width),static_cast<uint16_t>(height),static_cast<uint16_t>(channel),**imageView, **imageTemp,**deviceMem,**sampler,uploadBuffer,mem };
	}
}

