#include "vulkanInstance.h"
#include <limits>
namespace ltCore {

	void vulkanInstance::createSurface(vk::raii::SurfaceKHR& surface, GLFWwindow* window)
	{
		VkSurfaceKHR       _surface;
		if (glfwCreateWindowSurface(*m_instance, window, nullptr, &_surface) != 0) {
			throw std::runtime_error("failed to create window surface!");
		}
		surface = vk::raii::SurfaceKHR(m_instance, _surface);
	}
	void vulkanInstance::createInstance(std::string name, bool useValidationLayers)
	{
		vk::ApplicationInfo appInfo{ name.c_str(),VK_MAKE_VERSION(1, 0, 0),"LiteEngine",VK_MAKE_VERSION(1, 0, 0),vk::ApiVersion14 };
		std::vector<char const*> requiredLayers;
		if (useValidationLayers) {
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		auto layerProperties = m_context.enumerateInstanceLayerProperties();
		if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
			[requiredLayer](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			}))
		{
			throw std::runtime_error("One or more required layers are not supported!");
		}
		auto requiredExtensions = getRequiredInstanceExtensions(useValidationLayers);
		auto extensionProperties = m_context.enumerateInstanceExtensionProperties();
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

		/*std::vector<vk::ValidationFeatureEnableEXT> enables = {
	vk::ValidationFeatureEnableEXT::eGpuAssisted,
	vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot
		};

		vk::ValidationFeaturesEXT features = {};
		features.enabledValidationFeatureCount = static_cast<uint32_t>(enables.size());
		features.pEnabledValidationFeatures = enables.data();*/

		vk::InstanceCreateInfo createInfo{};
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
		createInfo.ppEnabledLayerNames = requiredLayers.data(),
			createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = requiredExtensions.data();
		//createInfo.pNext = &features;
		m_instance = vk::raii::Instance(m_context, createInfo);
	}
	std::vector<const char*> vulkanInstance::getRequiredInstanceExtensions(bool enableValidationLayers)
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

	void vulkanInstance::Init(std::string name)
	{
		bool usevalidation = true;
		std::vector<char const*> requiredLayers;
		if (usevalidation) {
			requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}
		auto layerProperties = m_context.enumerateInstanceLayerProperties();
		if (std::ranges::any_of(requiredLayers, [&layerProperties](auto const& requiredLayer) {
			return std::ranges::none_of(layerProperties,
			[requiredLayer](auto const& layerProperty)
				{ return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			}))
		{
			usevalidation = false;
		}
		createInstance(name, usevalidation);
		//vulkan only needs 1 instance. Ever. Period.
		//Period? you need a pad?
		if (usevalidation) {
			
			handler.requiredExtensions = requiredLayers;

			messenger.setupMessenger(m_instance);

			vk::DebugUtilsMessengerCallbackDataEXT callbackData{};
			callbackData.pMessage = "--- WELCOME TO LITENGINE ---";

			// Manually submit an info message to see if your breakpoint hits
			m_instance.submitDebugUtilsMessageEXT(
				vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo,
				vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral,
				callbackData
			);
		}
		//creates a surface for window 0
		GLFWwindow* tempWindow = glfwCreateWindow(200, 200, "tempWindow", nullptr, nullptr);
		createSurface(tempSurface, tempWindow);

		// only one discardable surface to check graphics devices
		//devices
		//device handler scans all devices

		if (handler.scanDevices(m_instance) > 0)
		{
			handler.tagDevices(tempSurface);
			handler.sortDevices();
		}
		else
		{
			//no devices, close engine
		}
		//if zero devices, exit
		if (tempWindow) {
			glfwDestroyWindow(tempWindow);
		}

		//only one command pool

		//create device pairs, one for each gpu to use
		//heres what you need multiple of for windows:
		//surfaces
		//pipelines
		//commandbuffer
		//sync stuff
	}
}
