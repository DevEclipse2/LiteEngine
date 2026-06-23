#include "Viewport.h"
#include "../EngineClasses/Lt_Gui.h"

namespace lte {

	void Viewport::SubmitGUICommands()
	{
		ImGui::Begin("Viewport",NULL);
		ImGui::Text("Free camera", NULL);
		ImGui::SliderFloat("Scale", &f, 0.1f, 5.0f);
		ImGui::Image(image.DS, ImVec2(image.image.width * f, image.image.height * f));
		ImGui::End();
		ImGui::Begin("Inspector", NULL);
		ImGui::Text("stuff", NULL);
		ImGui::End();
		ImGui::Begin("Profiler", NULL);
		ImGui::Text("idk", NULL);
		ImGui::End();


	}
	void Viewport::Init()
	{
		singleTimeCommandInfo info{ &Lt_Vulkan::devices[0].logicalDevice , &Lt_Vulkan::commandPool,&Lt_Vulkan::devices[0].queue };
		FileLoader::ImGUIImg("textures/texture.png", &image, Lt_Vulkan::devices[0].logicalDevice, Lt_Vulkan::devices[0].physicalDevice, info);
	}
	Viewport::Viewport()
	{
	}
	Viewport::~Viewport()
	{
		//des
		ImGui_ImplVulkan_RemoveTexture(image.DS);
	}
}