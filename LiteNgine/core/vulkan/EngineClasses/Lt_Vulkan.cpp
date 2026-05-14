#include "Lt_Vulkan.h"
#ifdef NDEBUG
#define noDbg false;
#else 
#define noDbg true;
#endif
namespace lte {
	vk::raii::Instance Lt_Vulkan::instance = nullptr;
	const std::vector<char const*> Lt_Vulkan::validationLayers = {
			"VK_LAYER_KHRONOS_validation"
	};
	vk::raii::Context Lt_Vulkan::context;
	std::vector<Lt_WindowVK> Lt_Vulkan::windows = {};
	std::vector<Lt_DevicePair> Lt_Vulkan::devices = {};
	vk::raii::SurfaceKHR Lt_Vulkan::TempSurface = nullptr;
	vk::raii::CommandPool Lt_Vulkan::commandPool = nullptr;
	vk::raii::Sampler Lt_Vulkan::sampler = nullptr;
	uint8_t Lt_Vulkan::FramesInFlight = 3;
	std::vector<void (*)()> Lt_Vulkan::resetFunction = {};
	std::vector<void (*)()> Lt_Vulkan::submitFunction = {};

	void Lt_Vulkan::createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window)
	{
		//connects the vulkan object with the window created by GLFW
		VkSurfaceKHR       _surface;
		if (glfwCreateWindowSurface(*instance,window, nullptr, &_surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		surface = vk::raii::SurfaceKHR(instance, _surface);
	}
	void Lt_Vulkan::Init(std::string name)
	{
		createInstance(name, true);
		//vulkan only needs 1 instance. Ever. Period.
		//Period? you need a pad?
		if (true) {
			messenger.setupMessenger(&instance);
		}
		//creates a surface for window 0
		GLFWwindow* tempWindow = glfwCreateWindow(200, 200, "tempWindow", nullptr, nullptr);
		createSurface(TempSurface,tempWindow);
		
		// only one discardable surface to check graphics devices
		//devices
		Lt_DevicePair devicepair{};
		DeviceHandler::pickPhysicalDevice(instance, devicepair.physicalDevice, devicepair.sampling);
		DeviceHandler::createLogicalDevice(devicepair.physicalDevice, devicepair.logicalDevice, TempSurface, devicepair.queue, devicepair.queueIndex, requiredDeviceExtensions);

		DeviceHandler::createTextureSampler(&sampler, devicepair.physicalDevice, devicepair.logicalDevice);
		devices.emplace_back(std::move(devicepair));
		if (tempWindow) {
			glfwDestroyWindow(tempWindow);
		}

		//only one command pool
		CommandBuffers::createCommandPool(&commandPool, &devices[0].logicalDevice, devices[0].queueIndex);

		//create device pairs, one for each gpu to use
		//heres what you need multiple of for windows:
		//surfaces
		//pipelines
		//commandbuffer
		//sync stuff
	}
	void Lt_Vulkan::createInstance(std::string name, bool useValidationLayers) 
	{
		vk::ApplicationInfo appInfo{ name.c_str(),VK_MAKE_VERSION(1, 0, 0),"LiteEngine",VK_MAKE_VERSION(1, 0, 0),vk::ApiVersion14 };
		std::vector<char const*> requiredLayers;
		if (useValidationLayers) {
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		auto layerProperties = context.enumerateInstanceLayerProperties();
		if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
				[requiredLayer](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			}))
		{
			throw std::runtime_error("One or more required layers are not supported!");
		}
		auto requiredExtensions = getRequiredInstanceExtensions(useValidationLayers);
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
	}
	std::vector<const char*> Lt_Vulkan::getRequiredInstanceExtensions(bool enableValidationLayers)
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
	Lt_WindowVK::Lt_WindowVK()
	{
		//do nothing
	}
	void Lt_WindowVK::registerWindow(uint32_t& windowIndex, uint32_t glfwWindowIndex)
	{
		windowIndex = Lt_Vulkan::windows.size();
		ltMultiWindowIndex = glfwWindowIndex;
		GLFWwindow* glfwWindow = Lt_WindowTracker::windowInfo[ltMultiWindowIndex]->window.getGLFWWindow();
		//new surface
		vk::raii::Device& device = Lt_Vulkan::devices[deviceID].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[deviceID].physicalDevice;
		Commands = {};
		Lt_Vulkan::createSurface(surface, glfwWindow);
		//new swapchain
		createSwapChain(Lt_WindowTracker::windowInfo[ltMultiWindowIndex]->window);
		//descriptor set layouts first
		PipelineDelegate::createDescriptorSetLayout(pipeline.descSetLayout, device);
		//a very fast pipeline creator
		//can be changed later
		PipelineDelegate::createPipelineFast(&pipeline, "shaders/slang.spv", "vertMain", "fragMain",device , PhysicalDevice, &swapchain.swapChainSurfaceFormat, pipeline.descSetLayout);
		//registers itself		

		/*Buffers::createUniformBuffers(&MeshInfo, framesInFlight, primary.device, PhysicalDevice);
		DeviceHandler::createDescriptorPool(&pool, &primary.device, maxObjects, framesInFlight);
		DeviceHandler::createDescriptorSets(pipeline.descSetLayout, pool, Lt_Vulkan::sampler, MeshInfo, framesInFlight, primary.device, renderSets);*/
		//CommandBuffers::createCommandBuffer(&commandBuffers, &Lt_Vulkan::commandPool, &device, Lt_Vulkan::FramesInFlight);
		
		//
		Lt_Vulkan::windows.emplace_back(std::move(this));

	}
	void Lt_WindowVK::createSwapChain(Lt_MultiWindow& window)
	{

		//LtSwapChain(vk::raii::PhysicalDevice & physicalDevice, vk::raii::Device & device, vk::raii::SurfaceKHR & surface, Lt_MultiWindow & window, uint32_t * minimgC)
		//swapchain = LtSwapChain{Lt_Vulkan::devices[deviceID]->physicalDevice}

		vk::raii::Device& device = Lt_Vulkan::devices[deviceID].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[deviceID].physicalDevice;
		vk::SampleCountFlagBits& msaaSamples = Lt_Vulkan::devices[deviceID].sampling;

		swapchain.createSwapChain(PhysicalDevice ,device,surface,window, &minImageCount);
		ImageDelegate::createSwapchainImageViews(&swapchain, &Lt_Vulkan::devices[deviceID].logicalDevice);
		LtImage colImg{};
		LtImage depImg{};
		ImageDelegate::createColorResources(&swapchain, colImg, device, PhysicalDevice,msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depImg, device, PhysicalDevice,msaaSamples);
		swapchain.colorImage = ImageDelegate::requestImageCreation(colImg);
		swapchain.depthImage = ImageDelegate::requestImageCreation(depImg);
		PipelineDelegate::createDescriptorSetLayout(pipeline.descSetLayout,device);
	}
	void Lt_WindowVK::recreateSwapChain() {


		width = 0;
		height = 0;

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(Lt_WindowTracker::windowInfo[ltMultiWindowIndex]->window.getGLFWWindow(), &width, &height);
			glfwWaitEvents();
		}
		Lt_Vulkan::devices[deviceID].logicalDevice.waitIdle();
		SwapchainHandler::cleanupSwapChain(&swapchain);
		//physical device is unlikely to change tbh
		//deviceHandler.pickPhysicalDevice(instance, PhysicalDevice, msaaSamples);
		//this frees the commandbuffers
		/*commandBuffers.clear();
		vertexBuffer = nullptr;
		indexBuffer = nullptr;


		deviceHandler.createLogicalDevice(PhysicalDevice, surface, primary, requiredDeviceExtensions);*/
		////do we really need to recreate the physical device??
		//swapchain = LtSwapChain{PhysicalDevice,primary.device,surface,&window,&minImageCount };

		vk::raii::Device& device = Lt_Vulkan::devices[deviceID].logicalDevice;
		vk::raii::PhysicalDevice& PhysicalDevice = Lt_Vulkan::devices[deviceID].physicalDevice;
		vk::SampleCountFlagBits& msaaSamples = Lt_Vulkan::devices[deviceID].sampling;
		ImageDelegate::createSwapchainImageViews(&swapchain, &device);
		ImageDelegate::requestImageDestruction(swapchain.colorImage);
		ImageDelegate::requestImageDestruction(swapchain.depthImage);
		LtImage colImg{};
		LtImage depImg{};
		ImageDelegate::createColorResources(&swapchain, colImg, device, PhysicalDevice, msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depImg, device, PhysicalDevice, msaaSamples);
		swapchain.colorImage = ImageDelegate::requestImageCreation(colImg);
		swapchain.depthImage = ImageDelegate::requestImageCreation(depImg);

		/*deviceHandler.createDescriptorPool(&pool, &primary.device, maxObjects, framesInFlight);
		deviceHandler.createDescriptorSets(pipeline.descSetLayout, pool, sampler, MeshInfo, framesInFlight, primary.device, renderSets);*/

	}
	void Lt_WindowVK::addCommand(vk::raii::CommandBuffer& commandBuffer)
	{
		Commands.emplace_back(*commandBuffer);
	}
	void Lt_WindowVK::resetBuffers()
	{
		Commands.clear();
	}
	void Lt_WindowVK::submitBuffers(uint8_t frameIndex)
	{
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		std::vector<vk::CommandBuffer> commands = {};
		commands.reserve(2 + Commands.size());
		//for safety pursposes :shrug:

		commands.insert(
			commands.end(),
			Commands.begin(),
			Commands.end()
		);

		//vkQueueSubmit() : pSubmits[0].pCommandBuffers[1] VkCommandBuffer 0x24b461d1cc8 is unrecorded and contains no commands.
		//	The Vulkan spec states : Each element of the pCommandBuffers member of each element of pSubmits must be in the pending or executable state(https ://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#VUID-vkQueueSubmit-pCommandBuffers-00070)
		//		Objects : 1
		//		[0] VkCommandBuffer 0x24b461d1cc8
		const vk::SubmitInfo submitInfo{
										1,
										//here
										&*syncSet.presentCompleteSemaphores[frameIndex],
										&waitDestinationStageMask,
										static_cast<uint32_t>(std::size(commands)),
										&*commands.data(),
										1,
										&*syncSet.renderFinishedSemaphores[frameIndex] };
		Lt_Vulkan::devices[0].queue.submit(submitInfo, *syncSet.inFlightFences[frameIndex]);
	}
}
