#include "Lt_ILayer.h"
#include "../EngineClasses/Preferences.h"
namespace lte {

	uint32_t Lt_ILayer::frameCount = 0;
	enum Result {
		Continue,
		Exit
	};

	void Lt_ILayer::Begin()
	{
		Con::Init();
		uint8_t result = 0;
		Bootstrapper::OnWake(&result);
		switch (result) {
		case Continue:
			Con::Log("program to continue execution", TAG_ENGINE);
			break;
		case Exit:
			Con::Log("requested program abort launch, terminating...", TAG_ENGINE);
			End();
			break;
		default:
			Con::LogError("unknown on wake result, possible programming oversight , please submit an issue on github,  interface layers / bootstrapper class", MED_SEVERITY, TAG_ENGINE);
			break;
		}
		Bootstrapper::DumpPreferences();
		Con::BootstrapDone();
		windowMgr.Startup();
		Lt_WindowInfo info;
			info.width = Preferences::Graphics::Width;
			info.height = Preferences::Graphics::Height;
			info.displayName = "LiteNgine editor";
			info.internalName = "MainWindow";
			info.resizePointers.emplace_back([this]() {
			this->Resize();
			});
		windowMgr.createMainWindow(info);
		vulkanHandler.Init("LiteNgine Editor");
		vk::raii::Device& device = Lt_Vulkan::devices[0].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[0].physicalDevice;
		singleTimeCommandInfo cmdInfo{ &device,&Lt_Vulkan::commandPool , &Lt_Vulkan::devices[0].queue};

		
		Lt_WindowVK mainWindow{};
		mainWindow.registerWindow(mainWindowIndex, windowMgr.mainId);
		Lt_Vulkan::windows.emplace_back(std::move(mainWindow));
		//load models and textures

		
		/*backend.InitializeVulkan(backendInfo);
		singleTimeCommandInfo cmdInfo{ &backend.primary.device ,&backend.commandPool , &backend.primary.queue };
		
		backend.second();*/

		Lt_GuiCreationInfo GuiCreationInfo{};
		GuiCreationInfo.width = info.width;
		GuiCreationInfo.height = info.height;
		GuiCreationInfo.cache = nullptr;
		//GuiCreationInfo.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		GuiCreationInfo.device = &device;
		GuiCreationInfo.instance = &Lt_Vulkan::instance;
		GuiCreationInfo.physicalDevice = &PhysicalDevice;
		GuiCreationInfo.queue = &Lt_Vulkan::devices[0].queue;
		GuiCreationInfo.minImgCount = Lt_Vulkan::windows[mainWindowIndex].minImageCount;
		GuiCreationInfo.queueFamily = Lt_Vulkan::devices[0].queueIndex;



		GuiCreationInfo.window = windowMgr.windowInfo[Lt_Vulkan::windows[mainWindowIndex].ltMultiWindowIndex]->window.getGLFWWindow();
		GuiCreationInfo.commandPool = &Lt_Vulkan::commandPool;
		GuiCreationInfo.maxFramesInFlight = Lt_Vulkan::FramesInFlight;
		GuiCreationInfo.pipeline = &Lt_Vulkan::windows[mainWindowIndex].pipeline.pipeline;
		GuiCreationInfo.layout = &Lt_Vulkan::windows[mainWindowIndex].pipeline.PipelineLayout;
		//theres a chance that it might override the original so im leaving this shit alone
		GuiCreationInfo.colorImageViewIndex = &Lt_Vulkan::windows[mainWindowIndex].swapchain.colorImage;
		GuiCreationInfo.pImageViews = &Lt_Vulkan::windows[mainWindowIndex].swapchain.imageViews;
		Con::LogEvent("Spinning up User Interface...", TAG_ENGINE);
		guiHandler.InitGui(GuiCreationInfo);
		guiHandler.Instantiate();			
		/*guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
		guiHandler.updateBuffers();*/
		Con::LogEvent("Init image viewport", TAG_ENGINE);
		viewport.Init();
		Con::LogEvent("Init editor viewport", TAG_ENGINE);

		std::cout << "Offset of colorImage: " + std::to_string(offsetof(EditorViewport, colorImage)) << std::endl;
		std::cout << "Offset of meshes: " + std::to_string(offsetof(EditorViewport, meshes)) << std::endl;
		std::cout << ("Address of this: " + std::to_string((uintptr_t)this)) << std::endl;
		editorViewport.Init(ImVec2(800,600),2);
		layoutloader.Init();
		//unfinished
		IridiumCFG config;
		Iridium::Init(config);
		Con::LogEvent("Preparing to render first frame", TAG_ENGINE);

	}
	void Lt_ILayer::Loop()
	{

		//auto miscGuiCommand = std::chrono::high_resolution_clock::now();

		Iridium::StartFrame(frameCount);
		Con::Display();

		//Con::LogEvent("NewFrame", TAG_ENGINE);
		Lt_Vulkan::windows[mainWindowIndex].newFrame(frames);
		Lt_Vulkan::windows[mainWindowIndex].resetBuffers();

		

		if (mainResized) {
			glfwGetFramebufferSize(windowMgr.windowInfo[Lt_Vulkan::windows[mainWindowIndex].ltMultiWindowIndex]->window.getGLFWWindow()
			, &Lt_Vulkan::windows[mainWindowIndex].width, &Lt_Vulkan::windows[mainWindowIndex].height);
			Lt_Vulkan::windows[mainWindowIndex].recreateSwapChain();
			guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
			mainResized = false;
			Con::LogEvent("main window resized", TAG_ENGINE);
		}
		
		//1 ms
		editorViewport.RenderScene(Lt_Vulkan::windows[mainWindowIndex].syncSet.presentCompleteSemaphores[frames]);
		//auto stopb = std::chrono::high_resolution_clock::now();

		//auto durationb = std::chrono::duration_cast<std::chrono::microseconds>(stopb - miscGuiCommand).count();
		//std::cout << "stuff took: " << (durationb / 1000.0f) << " ms\n";

		

		guiHandler.StartFrame();
		if(ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New")) { /* Handle New */ }
				if (ImGui::MenuItem("Open", "Ctrl+O")) { /* Handle Open */ }
				ImGui::Separator();
				if (ImGui::MenuItem("Exit", "Alt+F4")) { /* Handle Exit */ }

				ImGui::EndMenu();
			}
			//layoutloader.DrawMenu();
			ImGui::EndMainMenuBar();
		}
		//layoutloader.DrawPopups();

		Iridium::SubmitDrawCommands();

		//viewport.SubmitGUICommands();
		
		layoutloader.SubmitGUICommands();
		

		editorViewport.SubmitGUICommands();
		guiHandler.EndFrame();
		if (guiHandler.RenderFrame(frames)) 
		{
			guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
			guiHandler.updateBuffers();
			guiHandler.RenderFrame(frames);
		}
		//Con::Log("prepareCommandBuffers", TAG_ENGINE);
		Lt_Vulkan::windows[mainWindowIndex].prepCommand(frames);
		Lt_Vulkan::windows[mainWindowIndex].addCommand(guiHandler.commandBuffers[frames]);
		//Con::Log("submitCommandbuffer", TAG_ENGINE);
		Lt_Vulkan::windows[mainWindowIndex].submitBuffers(frames,editorViewport.syncSet.renderFinishedSemaphores[editorViewport.swapFrame]);
		//Con::Log("startRender", TAG_ENGINE);
		Lt_Vulkan::windows[mainWindowIndex].startRender(frames);
		if (frameCount == 20) {
			auto& deviceSet = Lt_Vulkan::devices[0];
			Con::LogEvent("CaptureFrame", TAG_ENGINE);
			ImageDelegate::DumpImages(deviceSet.logicalDevice, deviceSet.physicalDevice, *Lt_Vulkan::commandPool, *deviceSet.queue, Lt_Vulkan::windows[mainWindowIndex].swapchain.swapChainImages[frames], Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height, "swapchain.png");
		}
		
		editorViewport.FinishFrame();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();

		
		frameCount++;
		frames++;
		frames %= Lt_Vulkan::FramesInFlight;
		Iridium::EndFrame();
	}
	void Lt_ILayer::Resize()
	{
		Con::LogEvent("Resize called", TAG_ENGINE);
		int width = 0, height = 0;
		const auto& window = windowMgr.windowInfo[mainWindowIndex]->window.getGLFWWindow();
		glfwGetFramebufferSize(window, &width, &height);

		//this suspends the thread
		//we should probably use a bypass
		if (width == 0 || height == 0) {
			Con::LogEvent("Minimize", TAG_ENGINE);
		}
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents(); // Sleeps the thread until an event (like un-minimizing) occurs
		}
		mainResized = true;
	}
	void Lt_ILayer::End() 
	{
		Con::LogEvent("Engine Shutdown initiated", TAG_ENGINE);
		viewport.Terminate();
		guiHandler.Terminate();
		ImageDelegate::Terminate();
		Iridium::Terminate();
		/*
		backend.Exit();
		backend.window.DestroyWindow();*/
	}
	void Lt_ILayer::Cleanup()
	{
		Con::LogEvent("Engine Cleanup initiated", TAG_ENGINE);
		vulkanHandler.devices[0].logicalDevice.waitIdle();
		vulkanHandler.commandPool = nullptr;

	}
}
