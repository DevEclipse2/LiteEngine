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
	}
	void Lt_ILayer::Loop()
	{
		backend.Update();
		backend.Draw();
	}
	void Lt_ILayer::End() 
	{
		backend.Exit();
		backend.window.DestroyWindow();
	}
	void Lt_ILayer::Cleanup()
	{
	}
}
