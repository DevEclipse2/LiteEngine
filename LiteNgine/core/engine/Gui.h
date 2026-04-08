#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../vulkan/VulkanDevice.h"
namespace lte {
	class Gui
	{
	public:
		Gui();
		~Gui();
		void Init(VulkanDevice* device);
		void Destroy();

		VkCommandBuffer PrepareBuffer(uint32_t Image);
	private:
		void createDescriptorPool();
		void InitGUI();
		VulkanDevice* pDevice;
		int fbWidth;
		int fbHeight;
		std::vector<VkCommandBuffer> cmdBufs;
		VkDescriptorPool descriptorPool = NULL;

	};
}


