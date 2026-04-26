#include "Lt_ILayer.h"
namespace lte {

	void Lt_ILayer::Begin()
	{
		BackendInitInfo info{ 800,600,"LiteEngine : Agstrum",true,"LiteNgine editor" ,nullptr };
		backend.InitializeVulkan(info);
		singleTimeCommandInfo cmdInfo{ &backend.primary.device ,&backend.commandPool , &backend.primary.queue };
		fileLoader.TemporaryFileLoad(&backend.primary.device,&backend.PhysicalDevice, cmdInfo);
		backend.second();
	}
	void Lt_ILayer::Loop()
	{
		backend.Update();
	}
}
