#include "VulkanDevice.h"
#include "ConsoleLog.h"

namespace lte {

	VulkanDevice::VulkanDevice(Lt_Window& ltwind) : window{ ltwind }
	{
		//sets up the main vulkan context
		//vulkan instance
		createInstance();
		//debug messenger and validation layers
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();//throws validation error
		createCommandPool();
		createCommandBuffer();
		createSyncObjects();
	}
	VulkanDevice::~VulkanDevice() {

	}

	//validation layer stuff
	VKAPI_ATTR vk::Bool32 VKAPI_CALL VulkanDevice::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
		vk::DebugUtilsMessageTypeFlagsEXT              type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

		return vk::False;
	}

	//checks if debugging(if not , it quits
	void VulkanDevice::setupDebugMessenger() {
		//here
		if (!enableValidationLayers) return;
		//sets it up for both warnings AND errors using container
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		//warn types
		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		//this makes the message
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags,
		debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags,
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}




	void VulkanDevice::createSurface() 
	{
		//connects the vulkan object with the window created by GLFW
		VkSurfaceKHR       _surface;
		if (glfwCreateWindowSurface(*instance, window.getGLFWWindow(), nullptr, &_surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		surface = vk::raii::SurfaceKHR(instance, _surface);
	}
	std::vector<const char*> VulkanDevice::getRequiredInstanceExtensions()
	{
		//gets important extensions
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


		//scoring system i never bothered to implement
		//remind me to prioritize discrete gpus
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

		//lists all available features for funsies
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

		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan11Features,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT
		> featureChain{};

		//featureChain.get<vk::PhysicalDeviceFeatures2>() = nullptr;
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

	std::vector<char> VulkanDevice::readFile(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		std::vector<char> buffer(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();

		return buffer;
	}


	void VulkanDevice::createGraphicsPipeline() {

		//gets shaders for vert and frag respectively
		vk::raii::ShaderModule shaderModule = createShaderModule(readFile("shaders/slang.spv"));
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex,
			vertShaderStageInfo.module = shaderModule,
			vertShaderStageInfo.pName = "vertMain";
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment,
			fragShaderStageInfo.module = shaderModule,
			fragShaderStageInfo.pName = "fragMain";

		//defines pipeline

		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		vk::PipelineViewportStateCreateInfo      viewportState{};
		viewportState.viewportCount = 1, viewportState.scissorCount = 1;


		vk::PipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.depthClampEnable = vk::False,
			rasterizer.rasterizerDiscardEnable = vk::False,
			rasterizer.polygonMode = vk::PolygonMode::eFill,
			rasterizer.cullMode = vk::CullModeFlagBits::eBack,
			rasterizer.frontFace = vk::FrontFace::eClockwise,
			rasterizer.depthBiasEnable = vk::False,
			rasterizer.lineWidth = 1.0f;

		vk::PipelineMultisampleStateCreateInfo multisampling{};
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1,
			multisampling.sampleShadingEnable = vk::False;

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.blendEnable = vk::True,
			colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
			colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
			colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd,
			colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne,
			colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero,
			colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd,
			colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.logicOpEnable = vk::False,
			colorBlending.logicOp = vk::LogicOp::eCopy,
			colorBlending.attachmentCount = 1,
			colorBlending.pAttachments = &colorBlendAttachment;

		std::vector<vk::DynamicState>      dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			dynamicState.pDynamicStates = dynamicStates.data();


		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayoutCount = 0,
			pipelineLayoutInfo.pushConstantRangeCount = 0;

		pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

		vk::GraphicsPipelineCreateInfo graphicsInfo{};
			graphicsInfo.stageCount = 2,
			graphicsInfo.pStages = shaderStages,
			graphicsInfo.pVertexInputState = &vertexInputInfo,
			graphicsInfo.pInputAssemblyState = &inputAssembly,
			graphicsInfo.pViewportState = &viewportState,
			graphicsInfo.pRasterizationState = &rasterizer,
			graphicsInfo.pMultisampleState = &multisampling,
			graphicsInfo.pColorBlendState = &colorBlending,
			graphicsInfo.pDynamicState = &dynamicState,
			graphicsInfo.layout = pipelineLayout,
			graphicsInfo.renderPass = nullptr;

		vk::PipelineRenderingCreateInfo renderingInfo{};
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachmentFormats = &swapChainSurfaceFormat.format;

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain{graphicsInfo , renderingInfo};
		graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());

	}

	[[nodiscard]] vk::raii::ShaderModule VulkanDevice::createShaderModule(const std::vector<char>& code) const
	{

		vk::ShaderModuleCreateInfo createInfo{};
			createInfo.codeSize = code.size() * sizeof(char), 
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
			//createInfo.structureType = VK_KHR_shader_draw_parameters;
			vk::raii::ShaderModule shaderModule{device,createInfo}; //validationlayerError
			return shaderModule;
	}

	void VulkanDevice::createCommandPool() 
	{

		vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		poolInfo.queueFamilyIndex = queueIndex;
		commandPool = vk::raii::CommandPool(device, poolInfo);
	}

	void VulkanDevice::createCommandBuffer() {

		vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.commandPool = commandPool, 
			allocInfo.level = vk::CommandBufferLevel::ePrimary, 
			allocInfo.commandBufferCount = 1 ;
		commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());
	}

	void VulkanDevice::recordCommandBuffer(uint32_t imageIndex) {
		commandBuffer.begin({});
		transition_image_layout(
			imageIndex,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                         // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                 // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput          // dstStage
		);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::RenderingAttachmentInfo attachmentInfo = {};
			attachmentInfo.imageView = swapChainImageViews[imageIndex],
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear,
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore,
			attachmentInfo.clearValue = clearColor;

		vk::RenderingInfo renderingInfo = {};
			renderingInfo.renderArea = {.offset = { 0, 0 }, .extent = swapChainExtent },
			renderingInfo.layerCount = 1,
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachments = &attachmentInfo;
			
		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));
		commandBuffer.draw(3, 1, 0, 0);
		commandBuffer.endRendering();
		transition_image_layout(
			imageIndex,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe               // dstStage
		);
		commandBuffer.end();
	}

	void VulkanDevice::transition_image_layout(
		uint32_t imageIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask
	) 
	{
		vk::ImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor,
			subresourceRange.baseMipLevel = 0,
			subresourceRange.levelCount = 1,
			subresourceRange.baseArrayLayer = 0,
			subresourceRange.layerCount = 1;
		vk::ImageMemoryBarrier2 barrier = {};
			barrier.srcStageMask = srcStageMask,
			barrier.srcAccessMask = srcAccessMask,
			barrier.dstStageMask = dstStageMask,
			barrier.dstAccessMask = dstAccessMask,
			barrier.oldLayout = oldLayout,
			barrier.newLayout = newLayout,
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			barrier.image = swapChainImages[imageIndex],
			barrier.subresourceRange = subresourceRange;

			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.dependencyFlags = {},
			dependencyInfo.imageMemoryBarrierCount = 1,
			dependencyInfo.pImageMemoryBarriers = &barrier;

		commandBuffer.pipelineBarrier2(dependencyInfo);
	}

	void VulkanDevice::drawFrame() {
		auto fenceResult = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
		auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphore, nullptr);
		recordCommandBuffer(imageIndex);
		device.resetFences(*drawFence);
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo submitInfo{
			1, &* presentCompleteSemaphore, & waitDestinationStageMask, 1,
				&* commandBuffer, 1, &* renderFinishedSemaphore};
		queue.submit(submitInfo, *drawFence);
		result = device.waitForFences(*drawFence, vk::True, UINT64_MAX);
		if (result != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}

		const vk::PresentInfoKHR presentInfoKHR{1, &*renderFinishedSemaphore,1, &*swapChain,&imageIndex};
		result = queue.presentKHR(presentInfoKHR);
		switch (result)
		{
		case vk::Result::eSuccess:
			break;
		case vk::Result::eSuboptimalKHR:
			std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
			break;
		default:
			std::cerr << "what the fuck" << "\n";
			break;        // an unexpected result is returned!
		}
		
	}

	void VulkanDevice::createSyncObjects() {

		presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
		renderFinishedSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
		vk::FenceCreateInfo info{};
		info.flags = vk::FenceCreateFlagBits::eSignaled;
		drawFence = vk::raii::Fence(device, info);

	}

	void VulkanDevice::Exit() {
		device.waitIdle();
		//error during cleanup
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
