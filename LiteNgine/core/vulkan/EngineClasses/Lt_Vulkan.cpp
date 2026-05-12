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
	std::vector<std::unique_ptr<Lt_DevicePair>> Lt_Vulkan::devices = {};
	vk::raii::SurfaceKHR Lt_Vulkan::TempSurface = nullptr;
	vk::raii::CommandPool Lt_Vulkan::commandPool = nullptr;

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
		handler.pickPhysicalDevice(instance, devicepair.physicalDevice, devicepair.sampling);
		handler.createLogicalDevice(devicepair.physicalDevice, devicepair.logicalDevice, TempSurface, devicepair.queue, devicepair.queueIndex, requiredDeviceExtensions);
		devices.emplace_back(std::make_unique<Lt_DevicePair>(std::move(devicepair)));
		if (tempWindow) {
			glfwDestroyWindow(tempWindow);
		}

		//only one command pool
		CommandBuffers::createCommandPool(&commandPool, &devices[0]->logicalDevice, devices[0]->queueIndex);

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
	void Lt_WindowVK::registerWindow(uint32_t& windowIndex)
	{
		windowIndex = Lt_Vulkan::windows.size();

		Lt_Vulkan::createSurface(surface, Lt_MultiWindow::);



		//registers itself
		Lt_Vulkan::windows.emplace_back(std::make_unique<Lt_WindowVK>(std::move(this)));
	}
	void Lt_WindowVK::createSwapChain(char arguments, char deviceID)
	{

		swapchain = LtSwapChain{ Lt_Vulkan::devices[deviceID]->physicalDevice,Lt_Vulkan::devices[deviceID]->logicalDevice,surface,&window,&minImageCount};
		ImageDelegate::createSwapchainImageViews(&swapchain, &primary.device);
		LtImage colImg{};
		LtImage depImg{};
		ImageDelegate::createColorResources(&swapchain, colImg, primary.device, PhysicalDevice, msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depImg, primary.device, PhysicalDevice, msaaSamples);

		colorImageIndex = ImageDelegate::requestImageCreation(colImg);
		depthImageIndex = ImageDelegate::requestImageCreation(depImg);


		PipelineDelegate::createDescriptorSetLayout(pipeline.descSetLayout, primary.device);
	}
}
