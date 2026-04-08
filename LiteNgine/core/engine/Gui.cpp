#include "Gui.h"
namespace lte{
	void Gui::Init(VulkanDevice* device)
	{
		pDevice = device;
		pDevice->getFrameBufferSize(&fbWidth, &fbHeight);
		createDescriptorPool();
		InitGUI();
	}
	void Gui::createDescriptorPool() {
		VkDescriptorPoolSize poolSizes[] = {
			{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1000},
			{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ,1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ,1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ,1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER ,1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ,1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ,1000},
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ,1000},
			{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ,1000},
			{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT , 1000}
		};
		
		VkDescriptorPoolCreateInfo PoolCreateInfo = {};
			PoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			PoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			PoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
			PoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes),
			PoolCreateInfo.pPoolSizes = poolSizes;

			VkResult res = vkCreateDescriptorPool(pDevice->getDevice(), &PoolCreateInfo, NULL, &descriptorPool);;
			 
	}


	void Gui::InitGUI() {
		ImGui::CreateContext();
		ImGuiIO io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
		io.DisplaySize.x = (float)fbWidth;
		io.DisplaySize.y = (float)fbHeight;

		ImGui::GetStyle().FontScaleMain = 1.5f;
		ImGui::StyleColorsDark();
		bool InstallGlfwCallbacks = true;
		ImGui_ImplGlfw_InitForVulkan(pDevice->getWindow(), InstallGlfwCallbacks);
		VkFormat colorFormat = 
	}
}