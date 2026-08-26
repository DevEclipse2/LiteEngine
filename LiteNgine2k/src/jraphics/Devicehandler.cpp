#include "Devicehandler.h"
#include "../forScrap/Lt_Console.h"
#include <set>
#define compute		1
#define render		2
#define raytracing	4
#define present		8
#define discrete    16
#define geometry	32
#define dynamicRender 64
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
				scores[iterator].first -= 100;
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
				if (extName == vk::KHRDynamicRenderingExtensionName) {
					info.tags |= dynamicRender;
					scores[iterator].first += 700;
				}
				//if(extName == vk::KHRDynamicRenderingExtensionName)
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
		auto& context =	deviceContexts[index];
		context.physicalDeviceIndex = index;

		//here create stuff
		vk::DeviceCreateInfo createInfo{};
		void* currentPnext = nullptr;
		std::vector<std::vector<uint8_t>> structures = deduplicateCreationChains(index);
		void* pNextChain = nullptr;
		for (auto& buffer : structures) {
			auto* header = reinterpret_cast<VkBaseOutStructure*>(buffer.data());
			header->pNext = static_cast<VkBaseOutStructure*>(pNextChain);
			pNextChain = header;
		}
		//deviceContexts[index].logicalDevice

		std::map<uint32_t, uint8_t> familyToFlags;

		familyToFlags[devices[context.physicalDeviceIndex].graphicsFamily] |= graphicsBit;
		familyToFlags[devices[context.physicalDeviceIndex].computeFamily] |= computeBit;
		familyToFlags[devices[context.physicalDeviceIndex].transferFamily] |= transferBit;

		// 2. Build the DeviceQueueCreateInfos for unique families
		float queuePriority = 1.0f;
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(familyToFlags.size());

		for (const auto& [familyIndex, flags] : familyToFlags) {
			
			if (familyIndex == -1)
			{
				//incase of -1 indexes
				continue;
			}
			vk::DeviceQueueCreateInfo queueInfo{};
			queueInfo.queueFamilyIndex = familyIndex;
			queueInfo.queueCount = 1;
			queueInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueInfo);
		}

		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
		deviceCreateInfo.pNext = pNextChain;
		context.logicalDevice = vk::raii::Device(*m_physicalDevice[context.physicalDeviceIndex], deviceCreateInfo);
		context.m_Queue.clear();
		context.QueueFlagBits.clear();
		context.m_Queue.reserve(familyToFlags.size());
		context.QueueFlagBits.reserve(familyToFlags.size());

		for (const auto& [familyIndex, flags] : familyToFlags) {
			context.m_Queue.emplace_back(context.logicalDevice, familyIndex, 0);
			context.QueueFlagBits.push_back(flags);
		}
		//for queues, if theres a compute only queue or a transfer only queue,
	}
	std::vector<std::vector<uint8_t>> Devicehandler::deduplicateCreationChains(uint16_t index)
	{
		std::unordered_map<uint32_t, size_t> sTypeIndexMap; // Maps sType -> index in deduplicatedStructs
		std::vector<std::vector<uint8_t>> deduplicatedStructs;

		for (size_t i = 0; i < deviceCreationChains[index].size(); ++i)
		{
			const void* rawPtr = deviceCreationChains[index][i].first;
			size_t structSize = deviceCreationChains[index][i].second;

			if (!rawPtr || structSize < sizeof(VkBaseOutStructure)) {
				continue;
			}

			const auto* incomingHeader = static_cast<const VkBaseOutStructure*>(rawPtr);
			uint32_t type = incomingHeader->sType;

			auto it = sTypeIndexMap.find(type);
			if (it == sTypeIndexMap.end())
			{
				// First time encountering this sType: Clone the entire byte payload
				std::vector<uint8_t> buffer(structSize);
				std::memcpy(buffer.data(), rawPtr, structSize);

				// Clear the local pNext pointer before storing (engine will chain them later)
				auto* storedHeader = reinterpret_cast<VkBaseOutStructure*>(buffer.data());
				storedHeader->pNext = nullptr;

				sTypeIndexMap[type] = deduplicatedStructs.size();
				deduplicatedStructs.push_back(std::move(buffer));
			}
			else
			{
				// Duplicate detected: Merge with the existing buffer
				size_t targetIdx = it->second;
				std::vector<uint8_t>& existingBuffer = deduplicatedStructs[targetIdx];

				if (existingBuffer.size() != structSize)
				{
					//yeah this sucks
					lte::Con::LogError("Incorrect struct sizes during device creation chain deduplication!", CRIT_SEVERITY, TAG_VULKAN);
					continue;
				}

				// Offset past the sType (4B) + padding (4B) + pNext (8B) = 16 bytes
				constexpr size_t headerOffset = sizeof(VkBaseOutStructure);

				uint32_t* existingBools = reinterpret_cast<uint32_t*>(existingBuffer.data() + headerOffset);
				const uint32_t* incomingBools = reinterpret_cast<const uint32_t*>(
					static_cast<const uint8_t*>(rawPtr) + headerOffset
					);

				size_t boolCount = (structSize - headerOffset) / sizeof(uint32_t);

				for (size_t b = 0; b < boolCount; ++b) {
					existingBools[b] |= incomingBools[b]; // Combine flags with bitwise OR
				}
			}
		}

		return deduplicatedStructs;
	}
	void Devicehandler::createDevices()
	{
		//foreach device create them via requirements
		for (int i = 0; i < devices.size(); i++)
		{
			//deduplicate
			lte::Con::LogEvent("create logicalDevice" + i, TAG_ENGINE | TAG_VULKAN);
			createContext(i);
			//here free the memory

		}
	}
}

