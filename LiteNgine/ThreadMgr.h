#pragma once
#include<mutex>
namespace lte {
	class ThreadMgr
	{
	private: 
		std::mutex resourceMutex;
	};
}


