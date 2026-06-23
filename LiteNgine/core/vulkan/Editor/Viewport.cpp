#include "Viewport.h"
#include "../EngineClasses/Lt_Gui.h"

namespace lte {
	void Viewport::SubmitGUICommands()
	{
		ImGui::Begin("Viewport",NULL);
		ImGui::Text("Free camera", NULL);
		ImGui::SliderFloat("Scale", &f, 0.1f, 5.0f);
		ImGui::Image(DS, ImVec2(image.width, image.height));
		ImGui::End();
	}
	void Viewport::Init()
	{
		singleTimeCommandInfo info{ &Lt_Vulkan::devices[0].logicalDevice , &Lt_Vulkan::commandPool,&Lt_Vulkan::devices[0].queue };
		FileLoader::createTextureImage("textures/texture.png", image, Lt_Vulkan::devices[0].logicalDevice, Lt_Vulkan::devices[0].physicalDevice, info);
		
		ImageDelegate::transitionImageLayout(image.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, image.mipLevels, info);

		DS = ImGui_ImplVulkan_AddTexture(*image.imageSampler,*image.imageView,VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}