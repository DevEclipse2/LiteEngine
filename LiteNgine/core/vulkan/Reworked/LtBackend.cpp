#include "LtBackend.h"
namespace lte {
	LtBackend::LtBackend(BackendInitInfo info) : width{info.width} , height{info.height} , name{info.WindowName}
	{
		createInstance(info);
		if (info.useValidationLayers) {
			messenger.setupMessenger(&instance);
		}
		createSurface();
		deviceHandler.pickPhysicalDevice(&instance, &PhysicalDevice, &msaaSamples);
		deviceHandler.createLogicalDevice(&PhysicalDevice, &surface, &primary, &requiredDeviceExtensions);
		swapchain = LtSwapChain{ &PhysicalDevice,&primary.device,&surface,&window,&minImageCount };
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
}