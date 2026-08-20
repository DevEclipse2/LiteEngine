#include "Devicehandler.h"
#include "../forScrap/Lt_Console.h"
#define compute		1
#define render		2
#define raytracing	4
#define present		8
#define discrete    16
#define geometry	32
namespace ltCore
{
	int Devicehandler::scanDevices(const vk::raii::Instance& instance)
	{
		auto availableDevices = instance.enumeratePhysicalDevices();
		if (availableDevices.empty())
		{
			lte::Con::LogError("No Devices found with Vulkan Support", CRIT_SEVERITY, TAG_VULKAN);
			return 0;
		}
		for (const auto& device : availableDevices)
		{
			//registers deviceinfo,
			//adds 
			LtDeviceInfo info{};
			info.ID = m_physicalDevice.size();
			devices.push_back(info);
			m_physicalDevice.push_back(std::move(std::make_unique<vk::raii::PhysicalDevice>(device)));

		}
		return availableDevices.size();
	}
	void Devicehandler::tagDevices(const vk::raii::SurfaceKHR& surface)
	{
		int iterator = 0;
		for (const auto& device : m_physicalDevice)
		{
			auto& info = devices[iterator];
			scores.emplace_back(std::pair<int, uint32_t>(0, iterator));
			
			vk::raii::PhysicalDevice& physicalDevice = *device;
			vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
			auto queueFamilies = physicalDevice.getQueueFamilyProperties();
			auto extensions = physicalDevice.enumerateDeviceExtensionProperties();
			vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();
			vk::PhysicalDeviceMemoryProperties memProps = physicalDevice.getMemoryProperties();
			
			info.name = props.deviceName.data();
			uint64_t totalVramBytes = 0;
			for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
				if (memProps.memoryHeaps[i].flags & vk::MemoryHeapFlagBits::eDeviceLocal) {
					totalVramBytes += memProps.memoryHeaps[i].size;
				}
			}
			info.vramMB = (float)totalVramBytes / (1024.0f * 1024.0f);
			for (uint32_t i = 0; i < queueFamilies.size(); i++) {
				const auto& family = queueFamilies[i];
				if (family.queueFlags & vk::QueueFlagBits::eGraphics) {
					info.graphicsFamily = i;
					info.tags |= render;
					scores[iterator].first += 1000;
				}
				if (family.queueFlags & vk::QueueFlagBits::eCompute) {
					info.computeFamily = i;
					info.tags |= compute;
					scores[iterator].first += 1000;
				}
				if (physicalDevice.getSurfaceSupportKHR(i, *surface)) {
					info.tags |= present;
					scores[iterator].first += 1200;

				}
			}
			//general tags
			if (deviceFeatures.geometryShader)
			{
				info.tags |= geometry;
			}
			else
			{
				scores[iterator].first -= 3500;
			}
			if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			{
				info.tags |= discrete;
				scores[iterator].first += 2500;
			}
			uint32_t major = vk::versionMajor(props.apiVersion);
			uint32_t minor = vk::versionMinor(props.apiVersion);
			if (major >= 1 && minor >= 3) {
				scores[iterator].first += 5000;
			}
			scores[iterator].first += props.limits.maxPushConstantsSize;
			scores[iterator].first += (props.limits.maxImageDimension2D / 100);

			if (deviceFeatures.samplerAnisotropy) {
				scores[iterator].first += static_cast<uint32_t>(props.limits.maxSamplerAnisotropy * 10.0f);
			}
			if (deviceFeatures.tessellationShader) scores[iterator].first += 100;
			bool hasSwapchain = false;
			for (const auto& ext : extensions) {
				std::string extName = ext.extensionName.data();
				if (extName == vk::KHRSwapchainExtensionName) hasSwapchain = true;
				if (extName == vk::KHRRayTracingPipelineExtensionName) {
					info.tags |= raytracing;
					scores[iterator].first += 500;
				}
			}
			if (!hasSwapchain) {
				info.tags &= ~present;
				scores[iterator].first -= 1500;
			}
			iterator++;
		}
	}
	void Devicehandler::sortDevices()
	{
		//sort first, then use the ordering to order physical devices and ltdevice info 
		std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b) {
			if (a.first == b.first) {
				return a.second < b.second;
			}
			return a.first > b.first;
			});
		decltype(m_physicalDevice) sortedPhysicalDevices;
		decltype(devices) sortedDevices;

		sortedPhysicalDevices.reserve(m_physicalDevice.size());
		sortedDevices.reserve(devices.size());

		for (const auto& score : scores)
		{
			uint32_t originalIndex = score.second;

			sortedPhysicalDevices.push_back(std::move(m_physicalDevice[originalIndex]));
			sortedDevices.push_back(std::move(devices[originalIndex]));
		}

		m_physicalDevice = std::move(sortedPhysicalDevices);
		devices = std::move(sortedDevices);

		for (size_t i = 0; i < scores.size(); ++i) {
			scores[i].second = i;
		}
		for (size_t i = 0; i < devices.size(); ++i) {
			devices[i].ID = i;
		}
	}
	void Devicehandler::prepContexts()
	{
		deviceContexts.resize(devices.size());
	}
	void Devicehandler::createContext(uint16_t index)
	{
		if (index > deviceContexts.size())
		{
			lte::Con::LogError("Target gpu index > gpu array, check your work!", HIGH_SEVERITY, TAG_ENGINE | TAG_VULKAN);
			return;
		}
		//here create stuff

		//deviceContexts[index].logicalDevice

		//for queues, if theres a compute only queue or a transfer only queue,
	}
}

