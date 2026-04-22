#include "LtBackend.h"
namespace lte {
	LtBackend::LtBackend(BackendInitInfo info) : width{info.width} , height{info.height} , name{info.WindowName}
	{
		InitializeVulkan(info);
		//load models (or they are already loaded, i won't judge)
		deviceHandler.createTextureSampler(&sampler, &PhysicalDevice, &primary.device);
	}
	void LtBackend::createSurface()
	{
		//connects the vulkan object with the window created by GLFW
		VkSurfaceKHR       _surface;
		if (glfwCreateWindowSurface(*instance, window.getGLFWWindow(), nullptr, &_surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		surface = vk::raii::SurfaceKHR(instance, _surface);
	}
	void LtBackend::InitializeVulkan(BackendInitInfo info) 
	{
		createInstance(info);
		if (info.useValidationLayers) {
			messenger.setupMessenger(&instance);
		}
		createSurface();
		deviceHandler.pickPhysicalDevice(&instance, &PhysicalDevice, &msaaSamples);
		deviceHandler.createLogicalDevice(&PhysicalDevice, &surface, &primary, requiredDeviceExtensions);
		swapchain = LtSwapChain{ &PhysicalDevice,&primary.device,&surface,&window,&minImageCount };
		ImageDelegate::createSwapchainImageViews(&swapchain, &primary.device);
		colorImage = ImageDelegate::requestImageCreation();
		depthImage = ImageDelegate::requestImageCreation();
		ImageDelegate::createColorResources(&swapchain, colorImage, &primary.device, &PhysicalDevice, msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depthImage, &primary.device, &PhysicalDevice, msaaSamples);
		PipelineDelegate::createDescriptorSetLayout(&pipeline.descSetLayout, &primary.device);
		PipelineDelegate::createPipelineFast(&pipeline, "shaders/shader.slang", "VerticeShader", "FragmentShader", &primary.device, &PhysicalDevice, &swapchain.swapChainSurfaceFormat, pipeline.descSetLayout);
		CommandBuffers::createCommandPool(&commandPool, &primary.device, primary.queueIndex);
	}
	void LtBackend::createInstance(BackendInitInfo info) {

		vk::ApplicationInfo appInfo{ info.name.c_str(),VK_MAKE_VERSION(1, 0, 0),"LiteEngine",VK_MAKE_VERSION(1, 0, 0),vk::ApiVersion14};

		std::vector<char const*> requiredLayers;
		if (info.useValidationLayers) {
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
		auto requiredExtensions = getRequiredInstanceExtensions(info.useValidationLayers);

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
	}
	void LtBackend::RegisterGameObjects()
	{
		//here you load data and add them to draw
		//a bool is 8 bits large
		//a char holds 8 bits, but can store 8 on/offs
		// the 8 bits are 1 , 2 , 4, 8 , 16 , 32 , 64 , and 128
		
		int n = 1;
		bool LittleEndian = (*(char*)&n == 1);
		// little endian if true
		 

		for (uint32_t i = 0; i < sizeof(drawList) / sizeof(drawList[0]); i++) {


			//for each byte
			char draw = drawList[i];
			byte shiftC = 0;
			loadNextBit:
			if (shiftC & 0x08) goto exit;
			if (draw & 0b1) {
				//draw this
				/*prepareModels();
				createTextureImage(i, textures[i], &imagesArr[i], &imageMem[i]);
				createTextureImageView(&imagesArr[i]);
				loadModel(i, models[i]);*/
			}
			draw >> 1;
			shiftC++;
			goto loadNextBit;
			exit:
			//multiple textures
		}
	}
	std::vector<const char*> LtBackend::getRequiredInstanceExtensions(bool enableValidationLayers)
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
	void LtBackend::updateDrawCount()
	{
		//this gets how many bits are set using the literally most memory efficient method
		objects = 0;
		for (uint32_t i = 0; i < DrawList.size(); i++) {
			unsigned long long int draw = DrawList[i];
			objects += __builtin_popcountll(draw);
		}
	}
}