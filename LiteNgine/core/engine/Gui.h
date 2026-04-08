#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../vulkan/VulkanDevice.h"
#include <vulkan/vulkan_raii.hpp>
namespace lte {
	class Gui
	{
	public:
		Gui();
		~Gui();
		void Init(VulkanDevice* device);
		void Destroy();

		vk::CommandBuffer PrepareBuffer(uint32_t Image);
	private:
		void createDescriptorPool();
		void InitGUI();
		VulkanDevice* pDevice;
		int fbWidth;
		int fbHeight;
		std::vector<vk::CommandBuffer> cmdBufs;
		vk::raii::DescriptorPool descriptorPool = nullptr;

	};
}


