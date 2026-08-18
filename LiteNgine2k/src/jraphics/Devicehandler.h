#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>
// use tags
//tags for the gpu 
#define compute
#define render
#define raytracing
#define general
#define present
namespace ltCore
{
	struct LtDeviceInfo {
		//usage and queue index
		std::vector<std::pair<QueueUsage, uint16_t>> usages;
		uint8_t ID;
		uint8_t DeviceUsage;
	};
	enum class QueueUsage : uint8_t
	{
		Draw,
		Compute,
		Transfer
	};
	
	class Devicehandler
	{
	public:
		std::vector<uint32_t> scores;
		std::vector<std::unique_ptr<LtDeviceInfo>> devices;	
		//in proper destructor order
		std::vector<std::unique_ptr<vk::raii::PhysicalDevice>> m_physicalDevice = {};
		std::vector<std::unique_ptr<vk::raii::Device>> m_logicalDevice = {};
		std::vector<std::unique_ptr<vk::raii::Queue>> m_Queue = {};

		void scanDevices();
		void tagDevices();
		void gradeDevices();
	};
}


