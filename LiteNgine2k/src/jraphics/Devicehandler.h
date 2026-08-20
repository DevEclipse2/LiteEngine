#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
// use tags
//tags for the gpu 
#define computeBit	1
#define graphicsBit	2
#define transferBit	4
namespace ltCore
{
	struct LtDeviceInfo {
		//usage and queue index
		uint16_t ID;
		uint16_t tags;
		int32_t graphicsFamily = -1;
		int32_t computeFamily = -1;
		int32_t transferFamily = -1;
		uint32_t vramMB = 0;
		std::string name;
	};
	struct FrameData {
		vk::raii::CommandPool commandPool = nullptr;
		vk::raii::CommandBuffer commandBuffer = nullptr;
		vk::raii::Semaphore imageAvailableSemaphore = nullptr;
		vk::raii::Semaphore renderFinishedSemaphore = nullptr;
		vk::raii::Fence inFlightFence = nullptr;
	};
	struct DeviceContext 
	{
		uint16_t physicalDeviceIndex = -1;
		vk::raii::Device logicalDevice = nullptr;
		std::vector<vk::raii::Queue> m_Queue = {};
		std::vector<uint8_t> QueueFlagBits = {};
		std::array<FrameData, 3> FrameData;
	};
	class Devicehandler
	{
	public:
		std::vector<std::pair<int,uint32_t>> scores; //score index pair, so when its sorted the index remains
		std::vector<LtDeviceInfo> devices;	
		//in proper destructor order
		std::vector<std::unique_ptr<vk::raii::PhysicalDevice>> m_physicalDevice = {};
		std::vector<DeviceContext> deviceContexts = {};
		int scanDevices(const vk::raii::Instance& instance);
		void tagDevices(const vk::raii::SurfaceKHR& surface);
		//int getDeviceCompatible(uint16_t deviceOffset,)
		void sortDevices();
		void prepContexts();
		void createContext(uint16_t index);

	};
}


