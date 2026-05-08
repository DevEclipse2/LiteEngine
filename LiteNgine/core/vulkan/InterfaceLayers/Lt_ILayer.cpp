#include "Lt_ILayer.h"
namespace lte {

	void Lt_ILayer::Begin()
	{

		BackendInitInfo backendInfo{ 800,600,"LiteEngine : Agstrum",true,"LiteNgine editor" ,nullptr };
		Lt_WindowInfo info;
		info.width = 800,
			info.height = 600;
		info.displayName = "LiteNgine editor";
		info.internalName = "MainWindow";
		windowMgr.createMainWindow(info);
		vulkanHandler.Init("LiteNgine Editor");
		
		backend.InitializeVulkan(backendInfo);
		singleTimeCommandInfo cmdInfo{ &backend.primary.device ,&backend.commandPool , &backend.primary.queue };
		fileLoader.TemporaryFileLoad(backend.primary.device,backend.PhysicalDevice, cmdInfo);
		backend.second();
		Lt_GuiCreationInfo GuiCreationInfo{};
		GuiCreationInfo.width = info.width;
		GuiCreationInfo.height = info.height;
		GuiCreationInfo.cache = nullptr;
		//GuiCreationInfo.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		GuiCreationInfo.device = &backend.primary.device;
		GuiCreationInfo.instance = &backend.instance;
		GuiCreationInfo.physicalDevice = &backend.PhysicalDevice;
		GuiCreationInfo.queue = &backend.primary.queue;
		GuiCreationInfo.minImgCount = backend.minImageCount;
		GuiCreationInfo.queueFamily = backend.primary.queueIndex;
		GuiCreationInfo.window = backend.window.getGLFWWindow();
		GuiCreationInfo.commandPool = &backend.commandPool;
		GuiCreationInfo.maxFramesInFlight = backend.framesInFlight;
		guiHandler.InitGui(GuiCreationInfo);
	}
	void Lt_ILayer::Loop()
	{
		backend.Update();
		//add any gui draw commands here
		if (guiHandler.drawFrame(backend.frameIndex)) 
		{
			//update stuff
			guiHandler.updateFrameBuffer(backend.width, backend.height);
			guiHandler.updateBuffers();
		}
		backend.SubmitCommandBuffers();
		backend.Draw();
	}
	void Lt_ILayer::End() 
	{
		guiHandler.Terminate();
		backend.Exit();
		backend.window.DestroyWindow();
	}
	void Lt_ILayer::Cleanup()
	{
	}
}
