#include "../VulkanDevice.h"

namespace lte {

	bool VulkanDevice::isDeviceSuitable(vk::raii::PhysicalDevice const& physicalDevice)
	{

		vk::PhysicalDeviceProperties deviceProperties = physicalDevice.getProperties();
		vk::PhysicalDeviceFeatures deviceFeatures = physicalDevice.getFeatures();
		vk::PhysicalDeviceMemoryProperties deviceMemoryProperties = physicalDevice.getMemoryProperties();
		ListFeatures(&deviceProperties, &deviceFeatures, &deviceMemoryProperties);

		bool supportsVulkan1_3 = physicalDevice.getProperties().apiVersion >= vk::ApiVersion13;
		auto queueFamilies = physicalDevice.getQueueFamilyProperties();
		bool supportsGraphics =
			std::ranges::any_of(queueFamilies, [](auto const& qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); });

		auto availableDeviceExtensions = physicalDevice.enumerateDeviceExtensionProperties();
		bool supportsAllRequiredExtensions =
			std::ranges::all_of(requiredDeviceExtensions,
				[&availableDeviceExtensions](auto const& requiredDeviceExtension)
				{
					return std::ranges::any_of(availableDeviceExtensions,
						[requiredDeviceExtension](auto const& availableDeviceExtension)
						{ return strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0; });
				});
		auto features = physicalDevice.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
		bool supportsRequiredFeatures = 
			features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
			features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
			features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

		return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
		//return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures && deviceFeatures.samplerAnisotropy;
	}

	void VulkanDevice::pickPhysicalDevice() {

		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
		/*auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
		if (devIter == physicalDevices.end())
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		physicalDevice = *devIter;*/


		//scoring system i never bothered to implement
		//remind me to prioritize discrete gpus
		
		//std::vector<vk::raii::PhysicalDevice> physicalDevices = vk::raii::PhysicalDevices(instance);
		if (physicalDevices.empty())
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}
		if (physicalDevices.empty())
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, vk::raii::PhysicalDevice> candidates;

		for (const auto& pd : physicalDevices)
		{

			vk::PhysicalDeviceProperties deviceProperties = pd.getProperties();
			vk::PhysicalDeviceFeatures deviceFeatures = pd.getFeatures();
			vk::PhysicalDeviceMemoryProperties memprops = pd.getMemoryProperties();
			uint32_t score = 0;

			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
				score += 1000;
			}

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Application can't function without geometry shaders
			if (!deviceFeatures.geometryShader)
			{
				continue;
			}
			candidates.insert(std::make_pair(score, pd));
			ListFeatures(&deviceProperties , &deviceFeatures , &memprops);


		}

		// Check if the best candidate is suitable at all
		if (!candidates.empty() && candidates.rbegin()->first > 0)
		{
			physicalDevice = candidates.rbegin()->second;
			//std::cout(physicalDevice)
		}
		else
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}



	void VulkanDevice::createLogicalDevice() {

		//creates graphics queue
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
				physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
			{
				// found a queue family that supports both graphics and present
				queueIndex = qfpIndex;
				break;
			}
		}
		if (queueIndex == ~0)
		{
			throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
		}
		// Create a chain of feature structures
		// query for Vulkan 1.3 features
		vk::PhysicalDeviceFeatures deviceFeatures{ deviceFeatures.samplerAnisotropy = true };
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		> featureChain{};

		featureChain.get<vk::PhysicalDeviceFeatures2>().features = deviceFeatures;
		featureChain.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = VK_TRUE;
		featureChain.get<vk::PhysicalDeviceVulkan13Features>().synchronization2 = VK_TRUE;
		featureChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = VK_TRUE;
		featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = VK_TRUE;

		// create a Device
		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.queueFamilyIndex = queueIndex, deviceQueueCreateInfo.queueCount = 1, deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			deviceCreateInfo.queueCreateInfoCount = 1,
			deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo,
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
			deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		device = vk::raii::Device(physicalDevice, deviceCreateInfo);
		queue = vk::raii::Queue(device, queueIndex, 0);
	}

}