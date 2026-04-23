#include "Lt_ILayer.h"
namespace lte {

	void Lt_ILayer::Begin()
	{
		BackendInitInfo info{ 800,600,"LiteEngine : Agstrum",true,"LiteNgine editor" ,&ltWindow };
		backend.InitializeVulkan(info);
		
	}
}
