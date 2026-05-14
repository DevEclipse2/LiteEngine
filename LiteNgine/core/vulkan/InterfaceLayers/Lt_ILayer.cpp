#include "Lt_ILayer.h"
namespace lte {

	void Lt_ILayer::Begin()
	{
		windowMgr.Startup();
		Lt_WindowInfo info;
		info.width = 800,
			info.height = 600;
		info.displayName = "LiteNgine editor";
		info.internalName = "MainWindow";
		info.resizePointers.emplace_back([this]() {
			this->Resize();
			});
		windowMgr.createMainWindow(info);
		vulkanHandler.Init("LiteNgine Editor");


		vk::raii::Device& device = Lt_Vulkan::devices[0].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[0].physicalDevice;
		vk::SampleCountFlagBits& msaaSamples = Lt_Vulkan::devices[0].sampling;
		singleTimeCommandInfo cmdInfo{ &device,&Lt_Vulkan::commandPool , &Lt_Vulkan::devices[0].queue};
		fileLoader.TemporaryFileLoad(device, PhysicalDevice, cmdInfo);


		
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
		guiHandler.InitGui(GuiCreationInfo);
		guiHandler.Instantiate();			
		guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
		guiHandler.updateBuffers();
	}
	void Lt_ILayer::Loop()
	{
		
		//backend.Update();
		
		//leads to weird behaviour
		Lt_Vulkan::windows[mainWindowIndex].newFrame(frames);
		Lt_Vulkan::windows[mainWindowIndex].resetBuffers();
		
		if (mainResized) {
			glfwGetWindowSize(windowMgr.windowInfo[Lt_Vulkan::windows[mainWindowIndex].ltMultiWindowIndex]->window.getGLFWWindow()
			, &Lt_Vulkan::windows[mainWindowIndex].width, &Lt_Vulkan::windows[mainWindowIndex].height);
			Lt_Vulkan::windows[mainWindowIndex].recreateSwapChain();
			guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
			mainResized = false;
		}
		//add any gui draw commands here
		if (guiHandler.drawFrame(frames)) 
		{
			//update stuff			
			guiHandler.updateFrameBuffer(Lt_Vulkan::windows[mainWindowIndex].width, Lt_Vulkan::windows[mainWindowIndex].height);
			guiHandler.updateBuffers();
			guiHandler.drawFrame(frames);
		}
		/*if(!backend.AddAdditionalCommands(guiHandler.commandBuffers[backend.frameIndex])) {
			std::cerr << "cannot submit additional commands" << std::endl;
		}*/
		Lt_Vulkan::windows[mainWindowIndex].prepCommand(frames);
		Lt_Vulkan::windows[mainWindowIndex].addCommand(guiHandler.commandBuffers[frames]);
		Lt_Vulkan::windows[mainWindowIndex].submitBuffers(frames);
		Lt_Vulkan::windows[mainWindowIndex].startRender(frames);
		/*backend.SubmitCommandBuffers();
		backend.Draw();*/
		frames++;
		frames %= Lt_Vulkan::FramesInFlight;
	}
	void Lt_ILayer::Resize()
	{
		mainResized = true;
	}
	void Lt_ILayer::End() 
	{
		guiHandler.Terminate();
		
		
		/*
		backend.Exit();
		backend.window.DestroyWindow();*/
	}
	void Lt_ILayer::Cleanup()
	{
	}
}
