#include "VulkanDevice.h"
#include "ConsoleLog.h"

namespace lte {

	VulkanDevice::VulkanDevice(Lt_Window& ltwind) : window{ ltwind }
	{
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();

	}
	VulkanDevice::~VulkanDevice() {

	}

	VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDevice::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
		vk::DebugUtilsMessageTypeFlagsEXT              type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

		return vk::False;
	}

	void VulkanDevice::setupDebugMessenger() {
		if (!enableValidationLayers) return;
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags,
		debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags,
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}


	void VulkanDevice::createSurface() 
	{
		VkSurfaceKHR       _surface;
		if (glfwCreateWindowSurface(*instance, window.getGLFWWindow(), nullptr, &_surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		surface = vk::raii::SurfaceKHR(instance, _surface);
	}
	std::vector<const char*> VulkanDevice::getRequiredInstanceExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers)
		{
			extensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		return extensions;
	}
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
		bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
			features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
		return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
	}
	void VulkanDevice::pickPhysicalDevice() {

		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
		auto const devIter = std::ranges::find_if(physicalDevices, [&](auto const& physicalDevice) { return isDeviceSuitable(physicalDevice); });
		if (devIter == physicalDevices.end())
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
		physicalDevice = *devIter;

		/*
		std::vector<vk::raii::PhysicalDevice> physicalDevices = vk::raii::PhysicalDevices(instance);
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
		}*/
	}
	void VulkanDevice::ListFeatures(vk::PhysicalDeviceProperties* props ,
		vk::PhysicalDeviceFeatures* features , vk::PhysicalDeviceMemoryProperties* memProps) {
		ConsoleLog::printU32("apiVersion", props->apiVersion);
		ConsoleLog::printU64("vendorID", props->vendorID);
		ConsoleLog::printU64("deviceID", props->deviceID);
		ConsoleLog::printStr("deviceName", props->deviceName);
		std::cout << "deviceType: " << static_cast<uint32_t>(props->deviceType) << "\n";
		ConsoleLog::printByteArrayHex("pipelineCacheUUID", props->pipelineCacheUUID, VK_UUID_SIZE);

		const auto& lim = props->limits;
		ConsoleLog::printU32("maxImageDimension1D", lim.maxImageDimension1D);
		ConsoleLog::printU32("maxImageDimension2D", lim.maxImageDimension2D);
		ConsoleLog::printU32("maxImageDimension3D", lim.maxImageDimension3D);
		ConsoleLog::printU32("maxImageArrayLayers", lim.maxImageArrayLayers);
		ConsoleLog::printU32("maxUniformBufferRange", lim.maxUniformBufferRange);
		ConsoleLog::printU32("maxStorageBufferRange", lim.maxStorageBufferRange);
		ConsoleLog::printU32("maxPushConstantsSize", lim.maxPushConstantsSize);
		ConsoleLog::printU32("maxMemoryAllocationCount", lim.maxMemoryAllocationCount);
		ConsoleLog::printU32("maxBoundDescriptorSets", lim.maxBoundDescriptorSets);

		ConsoleLog::printU32("maxComputeWorkGroupCount[0]", lim.maxComputeWorkGroupCount[0]);
		ConsoleLog::printU32("maxComputeWorkGroupCount[1]", lim.maxComputeWorkGroupCount[1]);
		ConsoleLog::printU32("maxComputeWorkGroupCount[2]", lim.maxComputeWorkGroupCount[2]);
		ConsoleLog::printU32("maxComputeWorkGroupInvocations", lim.maxComputeWorkGroupInvocations);

		ConsoleLog::printU32("maxSamplerAllocationCount", lim.maxSamplerAllocationCount);
		ConsoleLog::printU32("maxSamplerAnisotropy", lim.maxSamplerAnisotropy);
		ConsoleLog::printU32("bufferImageGranularity", lim.bufferImageGranularity);

		vk::PhysicalDeviceFeatures feats = *features;
		std::cout << "core features sample:\n";
		std::cout << "  geometryShader: " << feats.geometryShader << "\n";
		std::cout << "  tessellationShader: " << feats.tessellationShader << "\n";
		std::cout << "  samplerAnisotropy: " << feats.samplerAnisotropy << "\n";
		std::cout << "  shaderInt64: " << feats.shaderInt64 << "\n";
		std::cout << "  fillModeNonSolid: " << feats.fillModeNonSolid << "\n";
		std::cout << "  multiDrawIndirect: " << feats.multiDrawIndirect << "\n";

		vk::PhysicalDeviceMemoryProperties memoryProperties = *memProps;
		ConsoleLog::printU32("memoryHeap count", memProps->memoryHeapCount);
		for (uint32_t i = 0; i < memProps->memoryHeapCount; ++i) {
			std::cout << "  heap[" << i << "].size: " << memProps->memoryHeaps[i].size << "\n";
			std::cout << "  heap[" << i << "].flags: " << static_cast<uint32_t>(memProps->memoryHeaps[i].flags) << "\n";
		}

		std::cout << "memoryType count: " << memProps->memoryTypeCount << "\n";
		for (uint32_t i = 0; i < memProps->memoryTypeCount; ++i) {
			std::cout << "  type[" << i << "].propertyFlags: " << static_cast<uint32_t>(memProps->memoryTypes[i].propertyFlags)
				<< " heapIndex: " << memProps->memoryTypes[i].heapIndex << "\n";
		}
	}

	void VulkanDevice::createLogicalDevice() {

		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		uint32_t queueIndex = ~0;
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

		// create a Device
		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
		deviceQueueCreateInfo.queueFamilyIndex = queueIndex, deviceQueueCreateInfo.queueCount = 1, deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		
		
		// Create a chain of feature structures
		// query for Vulkan 1.3 features

		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		> featureChain{};
		featureChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = VK_TRUE;
		featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = VK_TRUE;

		vk::DeviceCreateInfo deviceCreateInfo{};
			deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			deviceCreateInfo.queueCreateInfoCount = 1,
			deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo,
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
			deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
		device = vk::raii::Device(physicalDevice, deviceCreateInfo);
		queue = vk::raii::Queue(device, queueIndex, 0);

		vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
		std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(surface);
		std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(surface);

	}

	vk::SurfaceFormatKHR VulkanDevice::chooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& availableFormats)
	{
		const auto formatIt = std::ranges::find_if(
			availableFormats,
			[](const auto& format) { return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear; });
		return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	}

	void VulkanDevice::createSwapChain()
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
		swapChainExtent = chooseSwapExtent(surfaceCapabilities);
		uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);
		std::vector<vk::SurfaceFormatKHR> availableFormats = physicalDevice.getSurfaceFormatsKHR(*surface);
		swapChainSurfaceFormat = chooseSwapSurfaceFormat(availableFormats);

		std::vector<vk::PresentModeKHR> availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);
		vk::PresentModeKHR              presentMode = chooseSwapPresentMode(availablePresentModes);

		vk::SwapchainCreateInfoKHR swapChainCreateInfo{}; 
			swapChainCreateInfo.surface = *surface,
			swapChainCreateInfo.minImageCount = minImageCount,
			swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format,
			swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace,
			swapChainCreateInfo.imageExtent = swapChainExtent,
			swapChainCreateInfo.imageArrayLayers = 1,
			swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive,
			swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform,
			swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			swapChainCreateInfo.presentMode = chooseSwapPresentMode(availablePresentModes),
			swapChainCreateInfo.clipped = true;
			swapChainCreateInfo.oldSwapchain = nullptr;
		swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
		swapChainImages = swapChain.getImages();

	}
	vk::PresentModeKHR VulkanDevice::chooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& availablePresentModes)
	{
		assert(std::ranges::any_of(availablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
		return std::ranges::any_of(availablePresentModes,
			[](const vk::PresentModeKHR value) { return vk::PresentModeKHR::eMailbox == value; }) ?
			vk::PresentModeKHR::eMailbox:
			vk::PresentModeKHR::eFifo;
	}
	uint32_t VulkanDevice::chooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& surfaceCapabilities)
	{
		auto minImageCount = (std::max)(3u, surfaceCapabilities.minImageCount);
		if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount))
		{
			minImageCount = surfaceCapabilities.maxImageCount;
		}
		return minImageCount;

		
	}
	vk::Extent2D VulkanDevice::chooseSwapExtent(vk::SurfaceCapabilitiesKHR const& capabilities)
	{
		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		int w, h;
		glfwGetFramebufferSize(window.getGLFWWindow(), &w, &h);
		uint32_t width = static_cast<uint32_t>(w);
		uint32_t height = static_cast<uint32_t>(h);
		return {
			std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
			std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}

	void VulkanDevice::createImageViews() {
		assert(swapChainImageViews.empty());

		vk::ImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.viewType = vk::ImageViewType::e2D,
			imageViewCreateInfo.format = swapChainSurfaceFormat.format,
			imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

		for (auto& image : swapChainImages)
		{
			imageViewCreateInfo.image = image;
			swapChainImageViews.emplace_back(device, imageViewCreateInfo);
		}
	}

	void VulkanDevice::createGraphicsPipeline() {

	}

	int VulkanDevice::createInstance() {

		constexpr vk::ApplicationInfo appInfo{"LiteEngine Editor",VK_MAKE_VERSION(1, 0, 0),"LiteEngine",VK_MAKE_VERSION(1, 0, 0),vk::ApiVersion14};

		std::vector<char const*> requiredLayers;
		if (enableValidationLayers) {
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}

		// Check if the required layers are supported by the Vulkan implementation.
		auto layerProperties = context.enumerateInstanceLayerProperties();
		if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			}))
		{
			throw std::runtime_error("One or more required layers are not supported!");
		}

		// Get the required extensions.
		auto requiredExtensions = getRequiredInstanceExtensions();

		// Check if the required extensions are supported by the Vulkan implementation.
		auto extensionProperties = context.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt =
			std::ranges::find_if(requiredExtensions,
				[&extensionProperties](auto const& requiredExtension) {
					return std::ranges::none_of(extensionProperties,
						[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
				});
		if (unsupportedPropertyIt != requiredExtensions.end())
		{
			throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
		}
		
		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data(),
		createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();

		instance = vk::raii::Instance(context, createInfo);
		debugMessenger = nullptr;
		setupDebugMessenger();
	}
}