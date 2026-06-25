#include "Viewport.h"
#include "../EngineClasses/Lt_Gui.h"

namespace lte {

	namespace ed = ax::NodeEditor;

	void Viewport::SubmitGUICommands()
	{

		ImGui::Begin("Viewport",NULL);
		ImGui::Text("Free camera", NULL);
		ImGui::SliderFloat("Scale", &f, 0.1f, 5.0f);
		ImGui::Image(image.DS, ImVec2(image.image.width * f, image.image.height * f));
		ImGui::End();
		ImGui::Begin("Inspector", NULL);

		auto& io = ImGui::GetIO();

		ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

		ImGui::Separator();

		ed::SetCurrentEditor(m_Context);
		ed::Begin("My Editor", ImVec2(0.0, 0.0f));
		int uniqueId = 1;
		// Start drawing nodes.
		ed::BeginNode(uniqueId++);
		ImGui::Text("Node A");
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> In");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(uniqueId++, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();


		ed::BeginNode(uniqueId++);
		ImGui::Text("Node B");
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> I");
		ed::EndPin();
		ed::BeginPin(uniqueId++, ed::PinKind::Input);
		ImGui::Text("-> II");
		ed::EndPin();
		ImGui::SameLine();
		ed::BeginPin(uniqueId++, ed::PinKind::Output);
		ImGui::Text("Out ->");
		ed::EndPin();
		ed::EndNode();

		ed::End();
		ed::SetCurrentEditor(nullptr);

		
		ImGui::Text("stuff", NULL);
		ImGui::End();
		ImGui::Begin("Profiler", NULL);
		ImGui::Text("idk", NULL);
		ImGui::End();


	}
	void Viewport::Init()
	{
		ed::Config config;
		config.SettingsFile = "Simple.json";
		m_Context = ed::CreateEditor(&config);

		singleTimeCommandInfo info{ &Lt_Vulkan::devices[0].logicalDevice , &Lt_Vulkan::commandPool,&Lt_Vulkan::devices[0].queue };
		FileLoader::ImGUIImg("textures/texture.png", &image, Lt_Vulkan::devices[0].logicalDevice, Lt_Vulkan::devices[0].physicalDevice, info);
	}
	void Viewport::Terminate()
	{
		//des
		ed::DestroyEditor(m_Context);

		ImGui_ImplVulkan_RemoveTexture(image.DS);
	}
	Viewport::Viewport()
	{
	}
	Viewport::~Viewport()
	{
	}
}