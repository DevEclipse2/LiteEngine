#include "LtBackend.h"
namespace lte {
	LtBackend::LtBackend()
	{
		
		
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
		colorImageIndex = ImageDelegate::requestImageCreation();
		depthImageIndex = ImageDelegate::requestImageCreation();
		ImageDelegate::createColorResources(&swapchain, colorImageIndex, &primary.device, &PhysicalDevice, msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depthImageIndex, &primary.device, &PhysicalDevice, msaaSamples);
		PipelineDelegate::createDescriptorSetLayout(&pipeline.descSetLayout, &primary.device);
		PipelineDelegate::createPipelineFast(&pipeline, "shaders/shader.slang", "VerticeShader", "FragmentShader", &primary.device, &PhysicalDevice, &swapchain.swapChainSurfaceFormat, pipeline.descSetLayout);
		CommandBuffers::createCommandPool(&commandPool, &primary.device, primary.queueIndex);
	}

	void LtBackend::second() {
		//load models (or they are already loaded, i won't judge)
		deviceHandler.createTextureSampler(&sampler, &PhysicalDevice, &primary.device);
		singleTimeCommandInfo cmdinfo{ &primary.device ,&commandPool , &primary.queue};

		Buffers::createVertexBuffer(FileLoader::VertexesSize,FileLoader::VertexArray, &vertexBuffer, &vertexBufferMem, cmdinfo, &PhysicalDevice);
		Buffers::createIndexBuffer(FileLoader::IndicesSize, FileLoader::IndicesArray, &indexBuffer, &indexBufferMem, cmdinfo , &PhysicalDevice);

		renderSets = FileLoader::renderSets;

		MeshInfo.push_back(LtMeshInfo{});
		MeshInfo.push_back(LtMeshInfo{});
		MeshInfo.push_back(LtMeshInfo{});
		MeshInfo[0].position = { 0.0f, 0.0f, -1.0f };
		MeshInfo[0].rotation = { glm::radians(90.0f), 0.0f, 0.0f };
		MeshInfo[0].scale = { 1.1f, 1.1f,1.1f };

		MeshInfo[1].position = { -2.0f, 0.0f, -1.0f };
		MeshInfo[1].rotation = { 0.0f, 0.0f, 0.0f };
		MeshInfo[1].scale = { 1.45f, 1.45f, 1.45f };

		MeshInfo[2].position = { 2.0f, 0.0f, -1.0f };
		MeshInfo[2].rotation = { glm::radians(90.0f), 0.0f, 0.0f };
		MeshInfo[2].scale = { 0.85f, 0.85f, 0.85f };
		MeshInfo.resize(renderSets.size());

		Buffers::createUniformBuffers(&MeshInfo, framesInFlight , &primary.device, &PhysicalDevice);
		deviceHandler.createDescriptorPool(&pool, &primary.device, maxObjects, framesInFlight);
		deviceHandler.createDescriptorSets(&pipeline.descSetLayout, &pool, &sampler, &MeshInfo, framesInFlight, &primary.device, &renderSets);
		CommandBuffers::createCommandBuffer(&commandBuffers, &commandPool, &primary.device, framesInFlight);
		LtSync::createSyncObjects(&synchronizationSet, &swapchain, &primary.device , framesInFlight);
	}

	void LtBackend::Update()
	{
		
		TemporaryDraw::updateUniformBuffer(frameIndex,&swapchain,&MeshInfo);

		auto fenceResult = primary.device.waitForFences(*synchronizationSet.inFlightFences[frameIndex], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}

		//here
		auto& presentCompleteSemaphore = *synchronizationSet.presentCompleteSemaphores[frameIndex];
		auto [result, imageIndex] = swapchain.swapChain.acquireNextImage(UINT64_MAX, presentCompleteSemaphore, nullptr);
		
		availableIndex = imageIndex;
		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			recreateSwapChain();
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
		{
			assert(result == vk::Result::eTimeout || result == vk::Result::eNotReady);
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		primary.device.resetFences(*synchronizationSet.inFlightFences[frameIndex]);
		commandBuffers[frameIndex].reset();
		TemporaryDraw::recordCommandBuffer(imageIndex,frameIndex,&commandBuffers[frameIndex],&swapchain,ImageDelegate::GetImagePtr(colorImageIndex),ImageDelegate::GetImagePtr(depthImageIndex),&pipeline,&vertexBuffer,&indexBuffer,&vertexBufferMem,&indexBufferMem,&MeshInfo,&renderSets);

		/*if (gui->drawFrame()) {
			gui->updateBuffers();
		}
		gui->drawFrame();*/

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::CommandBuffer PackedBuffer[] = { *commandBuffers[frameIndex] /*pUiCommandBuffer->at(frameIndex)*/ };

		auto& presentCompleteSemaphores = *synchronizationSet.presentCompleteSemaphores[frameIndex];
		auto& renderFinishedSemaphores	= *synchronizationSet.renderFinishedSemaphores[frameIndex];
		auto& inFlightFence				= *synchronizationSet.inFlightFences[frameIndex];
		const vk::SubmitInfo submitInfo{
										1,
										//here
										&presentCompleteSemaphores,
										&waitDestinationStageMask,
										static_cast<uint32_t>(std::size(PackedBuffer)),
										&*PackedBuffer,
										1,
										&renderFinishedSemaphores};
		primary.queue.submit(submitInfo, inFlightFence);
	}
	void LtBackend::Draw() {

		const vk::PresentInfoKHR presentInfoKHR{ 1, &*synchronizationSet.renderFinishedSemaphores[availableIndex],1, &*swapchain.swapChain,&availableIndex };

		if (window.Resized)
		{
			window.Resized = false;
			/*gui->firstFrame = true;
			gui->updateFrameBuffer();*/
			recreateSwapChain();
			std::cout << "resize" << "\n";
			return;
		}


		vk::Result result = primary.queue.presentKHR(presentInfoKHR); //error here 

		switch (result)
		{
		case vk::Result::eSuccess:
			break;
		case vk::Result::eSuboptimalKHR:
			window.Resized = false;
			recreateSwapChain();
			std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
			break;
		default:
			window.Resized = false;
			recreateSwapChain();
			std::cerr << "what the fuck" << "\n";
			break;        // an unexpected result is returned!
		}
		frameNumber++;
		frameIndex = (frameIndex + 1) % framesInFlight;
	}
	void LtBackend::Exit() {
		SwapchainHandler::cleanupSwapChain(&swapchain);
	}

	void LtBackend::recreateSwapChain() 
	{


		width = 0;
		height = 0;

		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window.getGLFWWindow(), &width, &height);
			glfwWaitEvents();
		}
		primary.device.waitIdle();
		SwapchainHandler::cleanupSwapChain(&swapchain);
		deviceHandler.pickPhysicalDevice(&instance, &PhysicalDevice, &msaaSamples);
		deviceHandler.createLogicalDevice(&PhysicalDevice, &surface, &primary, requiredDeviceExtensions);
		swapchain = LtSwapChain{ &PhysicalDevice,&primary.device,&surface,&window,&minImageCount };
		ImageDelegate::createSwapchainImageViews(&swapchain, &primary.device);
		ImageDelegate::createColorResources(&swapchain, colorImageIndex, &primary.device, &PhysicalDevice, msaaSamples);
		ImageDelegate::createDepthResources(&swapchain, depthImageIndex, &primary.device, &PhysicalDevice, msaaSamples);
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
		 
		/*
		for (uint32_t i = 0; i < sizeof(drawList) / sizeof(drawList[0]); i++) {


			//for each byte
			char draw = drawList[i];
			char shiftC = 0;
			loadNextBit:
			if (shiftC & 0x08) goto exit;
			if (draw & 0b1) {
				//draw this
				/*prepareModels();
				createTextureImage(i, textures[i], &imagesArr[i], &imageMem[i]);
				createTextureImageView(&imagesArr[i]);
				loadModel(i, models[i]);
			}
			draw >> 1;
			shiftC++;
			goto loadNextBit;
			exit:
			//multiple textures
		}*/
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
		/*objects = 0;
		for (uint32_t i = 0; i < DrawList.size(); i++) {
			unsigned long long int draw = DrawList[i];
			objects += __builtin_popcountll(draw);
		}*/
	}
}