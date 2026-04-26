#include "DeviceHandler.h"
namespace lte {
	uint32_t DeviceHandler::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties, vk::raii::PhysicalDevice* physicalDevice)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice->getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}
		throw std::runtime_error("failed to find suitable memory type!");
	}
	void DeviceHandler::pickPhysicalDevice(vk::raii::Instance* instance, vk::raii::PhysicalDevice* physicalDevice, vk::SampleCountFlagBits* sampling) 
	{

		std::vector<vk::raii::PhysicalDevice> physicalDevices = instance->enumeratePhysicalDevices();
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


			//ListFeatures(&deviceProperties, &deviceFeatures, &memprops);


		}

		// Check if the best candidate is suitable at all
		if (!candidates.empty() && candidates.rbegin()->first > 0)
		{
			*physicalDevice = candidates.rbegin()->second;
			*sampling = getMaxUsableSampleCount(physicalDevice);
			//std::cout(physicalDevice)
		}
		else
		{
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	vk::SampleCountFlagBits DeviceHandler::getMaxUsableSampleCount(vk::raii::PhysicalDevice* physicalDevice) 
	{
		vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice->getProperties();

		vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
		if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
		if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
		if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
		if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
		if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

		return vk::SampleCountFlagBits::e1;
	}
	void DeviceHandler::createLogicalDevice(vk::raii::PhysicalDevice* physicalDevice, vk::raii::SurfaceKHR* surface , LogicalDevice* logicalDevice , std::vector<const char*> requiredExtensions)
	{

		//creates graphics queue
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice->getQueueFamilyProperties();

		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && (queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eCompute) &&
				physicalDevice->getSurfaceSupportKHR(qfpIndex, **surface))
			{

				// found a queue family that supports both graphics and present
				logicalDevice->queueIndex = qfpIndex;
				break;
			}

		}
		if (logicalDevice->queueIndex == ~0)
		{
			throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
		}
		// Create a chain of feature structures
		// query for Vulkan 1.3 features
		vk::PhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
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
		deviceQueueCreateInfo.queueFamilyIndex = logicalDevice->queueIndex, deviceQueueCreateInfo.queueCount = 1, deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
		vk::DeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			deviceCreateInfo.queueCreateInfoCount = 1,
			deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo,
			deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
			deviceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
		logicalDevice->device = vk::raii::Device(*physicalDevice, deviceCreateInfo);
		logicalDevice->queue = vk::raii::Queue(logicalDevice->device, logicalDevice->queueIndex, 0);
		//computeQueue = vk::raii::Queue(device,computeQueueIndex,0);
	}
	void DeviceHandler::createTextureSampler(vk::raii::Sampler* sampler, vk::raii::PhysicalDevice* physicalDevice, vk::raii::Device* device)
	{
		vk::PhysicalDeviceProperties properties = physicalDevice->getProperties();
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = vk::Filter::eLinear,
			samplerInfo.minFilter = vk::Filter::eLinear,
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear,
			samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat,
			samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
		samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat,
			samplerInfo.mipLodBias = 0.0f,
			samplerInfo.anisotropyEnable = vk::True,
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			samplerInfo.compareEnable = vk::False,
			samplerInfo.compareOp = vk::CompareOp::eAlways,
			samplerInfo.minLod = 0.0f,
			samplerInfo.maxLod = vk::LodClampNone,
			/*samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
			samplerInfo.unnormalizedCoordinates = vk::False;
			samplerInfo.compareEnable           = vk::False;
			samplerInfo.compareOp               = vk::CompareOp::eAlways;
			samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
			samplerInfo.mipLodBias              = 0.0f;
			samplerInfo.minLod                  = 0.0f;
			samplerInfo.maxLod                  = 0.0f;*/
			*sampler = vk::raii::Sampler(*device, samplerInfo);
	}
	void DeviceHandler::createDescriptorPool(vk::raii::DescriptorPool* descriptorPool, vk::raii::Device* device, uint32_t maxObjects, uint8_t maxFIF)
	{

		std::array poolSize{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, maxObjects * maxFIF),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxObjects * maxFIF)
		};
		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			poolInfo.maxSets = maxObjects * maxFIF,
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
			poolInfo.pPoolSizes = poolSize.data();
		*descriptorPool = vk::raii::DescriptorPool(*device, poolInfo);
	}
	void DeviceHandler::createDescriptorSets(vk::DescriptorSetLayout* descriptorSetLayout, vk::raii::DescriptorPool* descriptorPool,vk::raii::Sampler* sampler, std::vector<LtMeshInfo>* meshes, uint8_t maxFIF , vk::raii::Device* device , std::vector<RenderSet>* rs)
	{

		uint32_t meshNo = 0;
		for (auto& gameObject : *meshes)
		{
			// Create descriptor sets for each frame in flight
			std::vector<vk::DescriptorSetLayout> layouts(maxFIF, *descriptorSetLayout);
			vk::DescriptorSetAllocateInfo allocInfo{};
			allocInfo.descriptorPool = **descriptorPool,
				allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
				allocInfo.pSetLayouts = layouts.data();

			gameObject.descriptorSets.clear();
			gameObject.descriptorSets = device->allocateDescriptorSets(allocInfo);
			for (size_t i = 0; i < maxFIF; i++) {

				vk::DescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = *gameObject.uniformBuffers[i],
					bufferInfo.offset = 0,
					bufferInfo.range = sizeof(UniformBufferObject);

				vk::DescriptorImageInfo imageInfo{};
				imageInfo.sampler = **sampler,
					imageInfo.imageView = (ImageDelegate::ImagePool[rs->at(meshNo).imageIndex])->imageView,
					imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;


				vk::WriteDescriptorSet descriptorbuffer{};
				descriptorbuffer.dstSet = *gameObject.descriptorSets[i],
					descriptorbuffer.dstBinding = 0,
					descriptorbuffer.dstArrayElement = 0,
					descriptorbuffer.descriptorCount = 1,
					descriptorbuffer.descriptorType = vk::DescriptorType::eUniformBuffer,
					descriptorbuffer.pBufferInfo = &bufferInfo;
				vk::WriteDescriptorSet descriptorimage{};
				descriptorimage.dstSet = *gameObject.descriptorSets[i],
					descriptorimage.dstBinding = 1,
					descriptorimage.dstArrayElement = 0,
					descriptorimage.descriptorCount = 1,
					descriptorimage.descriptorType = vk::DescriptorType::eCombinedImageSampler,
					descriptorimage.pImageInfo = &imageInfo;
				std::array descriptorWrites{
					descriptorbuffer,
					descriptorimage
				};

				device->updateDescriptorSets(descriptorWrites, {});
			}
			meshNo++;
		}
	}
}