#include "VulkanDevice.h"
#include "ConsoleLog.h"

#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
namespace lte {

	VulkanDevice::VulkanDevice(Lt_Window* ltwind) : window { *ltwind }
	{
		
		//vulkan instance
		createInstance();
		//debug messenger and validation layers
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		msaaSamples = getMaxUsableSampleCount();
		createSwapChain();
		createImageViews();
		createColorResources();
		createDepthResources();
		createDescriptorSetLayout();
		createGraphicsPipeline();//throws validation error
		createCommandPool();
		createTextureImage();
		createTextureImageView();
		createTextureSampler();
		prepareModels();
		for (int i = 0; i < models.size(); i++) {
			loadModel(i);
		}
		createVertexBuffer();
		createIndexBuffer();
		setupMeshes();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
		createCommandBuffer();
		createSyncObjects();


		

	}
	VulkanDevice::~VulkanDevice() {
		device.waitIdle();
		//cleanupSwapChain();
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
		debugUtilsMessengerCreateInfoEXT.messageSeverity	= severityFlags,
		debugUtilsMessengerCreateInfoEXT.messageType		= messageTypeFlags,
		debugUtilsMessengerCreateInfoEXT.pfnUserCallback	= &debugCallback;
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
	void VulkanDevice::createImageViews() {
		assert(swapChainImageViews.empty());

		vk::ImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.viewType			= vk::ImageViewType::e2D,
			imageViewCreateInfo.format				= swapChainSurfaceFormat.format,
			imageViewCreateInfo.subresourceRange	= { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };

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
	void VulkanDevice::createDescriptorSetLayout() {

		std::array bindings = {
	vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
	vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
		};

		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.size(), bindings.data());
		descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
		/*
		
			
		std::array bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
		};
		vk::DescriptorSetLayoutBinding uboLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr);
		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
			layoutInfo.bindingCount = 1, 
			layoutInfo.pBindings = &uboLayoutBinding;
		descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);*/
	}


	void VulkanDevice::createGraphicsPipeline() {

		//gets shaders for vert and frag respectively
		vk::raii::ShaderModule shaderModule = createShaderModule(shaderLoader.readFile("shaders/slang.spv"));
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
			vertShaderStageInfo.stage	= vk::ShaderStageFlagBits::eVertex,
			vertShaderStageInfo.module	= shaderModule,
			vertShaderStageInfo.pName	= "vertMain";
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.stage		= vk::ShaderStageFlagBits::eFragment,
			fragShaderStageInfo.module	= shaderModule,
			fragShaderStageInfo.pName	= "fragMain";
		//defines pipeline

		vk::PipelineShaderStageCreateInfo shaderStages[]				= { vertShaderStageInfo, fragShaderStageInfo };

		auto                                   bindingDescription		= Vertex::getBindingDescription();
		auto                                   attributeDescriptions	= Vertex::getAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.vertexBindingDescriptionCount					= 1,
			vertexInputInfo.pVertexBindingDescriptions					= &bindingDescription,
			vertexInputInfo.vertexAttributeDescriptionCount				= static_cast<uint32_t>(attributeDescriptions.size()),
			vertexInputInfo.pVertexAttributeDescriptions				= attributeDescriptions.data();


		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		vk::PipelineViewportStateCreateInfo      viewportState{};
		viewportState.viewportCount = 1, viewportState.scissorCount		= 1;


		//vk::PipelineRasterizationStateCreateInfo rasterizer{};
		vk::PipelineRasterizationStateCreateInfo rasterizer({}, vk::False, vk::False, vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, 0.0f, 0.0f, 1.0f, 1.0f);
		/*rasterizer.depthClampEnable = vk::False,
			rasterizer.rasterizerDiscardEnable = vk::False,
			rasterizer.polygonMode = vk::PolygonMode::eFill,
			rasterizer.cullMode = vk::CullModeFlagBits::eBack,
			rasterizer.frontFace = vk::FrontFace::eClockwise,
			rasterizer.depthBiasEnable = vk::False,
			rasterizer.lineWidth = 1.0f;
		*/
		vk::PipelineMultisampleStateCreateInfo multisampling{};
			multisampling.rasterizationSamples							= msaaSamples,
			multisampling.sampleShadingEnable							= vk::False;
		vk::PipelineDepthStencilStateCreateInfo depthStencil{};
			depthStencil.depthTestEnable								= vk::True,
			depthStencil.depthWriteEnable								= vk::True,
			depthStencil.depthCompareOp									= vk::CompareOp::eLess,
			depthStencil.depthBoundsTestEnable							= vk::False,
			depthStencil.stencilTestEnable								= vk::False;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.blendEnable								= vk::True,
			colorBlendAttachment.srcColorBlendFactor					= vk::BlendFactor::eSrcAlpha,
			colorBlendAttachment.dstColorBlendFactor					= vk::BlendFactor::eOneMinusSrcAlpha,
			colorBlendAttachment.colorBlendOp							= vk::BlendOp::eAdd,
			colorBlendAttachment.srcAlphaBlendFactor					= vk::BlendFactor::eOne,
			colorBlendAttachment.dstAlphaBlendFactor					= vk::BlendFactor::eZero,
			colorBlendAttachment.alphaBlendOp							= vk::BlendOp::eAdd,
			colorBlendAttachment.colorWriteMask							= vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		vk::PipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.logicOpEnable										= vk::False,
			colorBlending.logicOp										= vk::LogicOp::eCopy,
			colorBlending.attachmentCount								= 1,
			colorBlending.pAttachments									= &colorBlendAttachment;

		std::vector<vk::DynamicState>      dynamicStates				= { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
		vk::PipelineDynamicStateCreateInfo dynamicState{};
			dynamicState.dynamicStateCount								= static_cast<uint32_t>(dynamicStates.size()),
			dynamicState.pDynamicStates									= dynamicStates.data();


			/*vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
				pipelineLayoutInfo.setLayoutCount = 0,
				pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);*/
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.setLayoutCount = 1,
			pipelineLayoutInfo.pSetLayouts = &*descriptorSetLayout,
			pipelineLayoutInfo.pushConstantRangeCount = 0;

		pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

		vk::Format depthFormat = findDepthFormat();

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
			graphicsInfo.pDepthStencilState = &depthStencil,
			graphicsInfo.layout = pipelineLayout,
			graphicsInfo.renderPass = nullptr;

		vk::PipelineRenderingCreateInfo renderingInfo{};
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachmentFormats = &swapChainSurfaceFormat.format,
			renderingInfo.depthAttachmentFormat = depthFormat;

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

	void VulkanDevice::createVertexBuffer() {
		//assert(vertexBuffer == nullptr);

		vk::DeviceSize bufferSize =  0;
		for (const auto& inner : vertices) {
			bufferSize += sizeof(Vertex) * inner.size();
		}

		vk::BufferCreateInfo stagingInfo{};
			stagingInfo.size = bufferSize,
			stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc, 
			stagingInfo.sharingMode = vk::SharingMode::eExclusive;
		vk::raii::Buffer stagingBuffer(device, stagingInfo);
		vk::raii::DeviceMemory stagingBufferMemory({});

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
		//stagingBuffer.bindMemory(stagingBufferMemory, 0);
		void* dataStaging = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(dataStaging, vertices[0].data(), bufferSize);
		stagingBufferMemory.unmapMemory();

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);

		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		/*vk::BufferCreateInfo bufferInfo{};
			bufferInfo.size = bufferSize,
			bufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		vertexBuffer = vk::raii::Buffer(device, bufferInfo);

		vk::MemoryRequirements memRequirements = vertexBuffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memoryAllocateInfo{};
			memoryAllocateInfo.allocationSize = memRequirements.size,
			memoryAllocateInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
		vertexBufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);

		vertexBuffer.bindMemory(*vertexBufferMemory, 0);

		copyBuffer(stagingBuffer, vertexBuffer, stagingInfo.size);
		*/
	}

	void VulkanDevice::createIndexBuffer() {

		vk::DeviceSize bufferSize = 0;
		for (const auto& inner : indices) {
			bufferSize += sizeof(uint32_t) * inner.size();
		}

		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory stagingBufferMemory({});
		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

		void* data = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(data, indices.data(), (size_t)bufferSize);
		stagingBufferMemory.unmapMemory();

		createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);

		copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	}

	void VulkanDevice::copyBuffer(vk::raii::Buffer& srcBuffer, vk::raii::Buffer& dstBuffer, vk::DeviceSize size) {
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = commandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = 1;
		vk::raii::CommandBuffer commandCopyBuffer = std::move(device.allocateCommandBuffers(allocInfo).front());
		vk::CommandBufferBeginInfo cmdbuffer{};
		cmdbuffer.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		commandCopyBuffer.begin(cmdbuffer);
		commandCopyBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
		commandCopyBuffer.end();
		vk::SubmitInfo info{};
			info.commandBufferCount = 1,
			info.pCommandBuffers = &*commandCopyBuffer;
		queue.submit(info, nullptr);
		
		queue.waitIdle();
	}
	/*
	void VulkanDevice::updateVertexBuffer() {
		const vk::DeviceSize newSize =
			sizeof(testAnim.vertices[0]) * testAnim.vertices.size();
		void* data = vertexBufferMemory.mapMemory(0, newSize);
		std::memcpy(data, testAnim.vertices.data(),
			static_cast<size_t>(newSize));
		vertexBufferMemory.unmapMemory();		


		vk::DeviceSize bufferSize = sizeof(vertexHandler.vertices[0]) * vertexHandler.vertices.size();

		vk::BufferCreateInfo stagingInfo{};
		stagingInfo.size = bufferSize;
		stagingInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		stagingInfo.sharingMode = vk::SharingMode::eExclusive;

		vk::raii::Buffer stagingBuffer(device, stagingInfo);

		vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memoryAllocateInfoStaging{};
		memoryAllocateInfoStaging.allocationSize = memRequirementsStaging.size;
		memoryAllocateInfoStaging.memoryTypeIndex =
			findMemoryType(memRequirementsStaging.memoryTypeBits,
				vk::MemoryPropertyFlagBits::eHostVisible |
				vk::MemoryPropertyFlagBits::eHostCoherent);

		vk::raii::DeviceMemory stagingBufferMemory(device, memoryAllocateInfoStaging);

		stagingBuffer.bindMemory(stagingBufferMemory, 0);

		// --- Write new vertex data into staging ---
		void* mapped = stagingBufferMemory.mapMemory(0, bufferSize);
		memcpy(mapped, vertexHandler.vertices.data(), bufferSize);
		copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	}
	*/

	void VulkanDevice::updateUniformBuffer(uint32_t frame) 
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		
		std::cout << "fps: " << 1 / (time - prevtime) << "Delta :" << (time-prevtime) * 1000 << "miliseconds" << '\n';
		prevtime = time;
		UniformBufferObject ubo{};

		glm::mat4 view = glm::lookAt(glm::vec3(2.0f, -6.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height),
			0.1f, 20.0f);

		ubo.proj[1][1] *= -1;


		// Update uniform buffers for each object
		for (auto& gameObject : meshes) {
			// Apply continuous rotation to the object
			gameObject.rotation.y += 0.001f; // Slow rotation around Y axis

			// Get the model matrix for this object
			glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 model = gameObject.getModelMatrix() * initialRotation;

			// Create and update the UBO
			UniformBufferObject ubo{};
				ubo.model = model,
				ubo.view = view,
				ubo.proj = proj;

			// Copy the UBO data to the mapped memory
			memcpy(gameObject.uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
		}
	}
	void VulkanDevice::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::raii::Buffer& buffer, vk::raii::DeviceMemory& bufferMemory) {
		vk::BufferCreateInfo bufferInfo{};
			bufferInfo.size = size,
			bufferInfo.usage = usage,
			bufferInfo.sharingMode = vk::SharingMode::eExclusive;
		buffer = vk::raii::Buffer(device, bufferInfo);
		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{};
			allocInfo.allocationSize = memRequirements.size,
			allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		bufferMemory = vk::raii::DeviceMemory(device, allocInfo);
		buffer.bindMemory(*bufferMemory, 0);
	}

	uint32_t VulkanDevice::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) 
	{
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();
		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void VulkanDevice::createUniformBuffers() {

		for (auto& gameObject : meshes) {
			gameObject.uniformBuffers.clear();
			gameObject.uniformBuffersMemory.clear();
			gameObject.uniformBuffersMapped.clear();

			// Create uniform buffers for each frame in flight
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
				vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
				vk::raii::Buffer buffer({});
				vk::raii::DeviceMemory bufferMem({});
				createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
					vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
					buffer, bufferMem);
				gameObject.uniformBuffers.emplace_back(std::move(buffer));
				gameObject.uniformBuffersMemory.emplace_back(std::move(bufferMem));
				gameObject.uniformBuffersMapped.emplace_back(gameObject.uniformBuffersMemory[i].mapMemory(0, bufferSize));
			}
		}

		/*uniformBuffers.clear();
		uniformBuffersMemory.clear();
		uniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer buffer({});
			vk::raii::DeviceMemory bufferMem({});
			createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
			uniformBuffers.emplace_back(std::move(buffer));
			uniformBuffersMemory.emplace_back(std::move(bufferMem));
			uniformBuffersMapped.emplace_back(uniformBuffersMemory[i].mapMemory(0, bufferSize));
		}*/
	}

	void VulkanDevice::createDescriptorPool() {

		std::array poolSize{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT)
		};
		vk::DescriptorPoolCreateInfo poolInfo{};
			poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			poolInfo.maxSets = MAX_OBJECTS * MAX_FRAMES_IN_FLIGHT,
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
			poolInfo.pPoolSizes = poolSize.data();
		descriptorPool = vk::raii::DescriptorPool(device, poolInfo);




		/*std::array poolSize{
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
		};
		vk::DescriptorPoolCreateInfo poolInfo{
			poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT,
			poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
			poolInfo.pPoolSizes = poolSize.data()};
		descriptorPool = vk::raii::DescriptorPool(device, poolInfo);*/
	}

	void VulkanDevice::createDescriptorSets() {

		for (auto& gameObject : meshes) 
		{
			// Create descriptor sets for each frame in flight
			std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
			vk::DescriptorSetAllocateInfo allocInfo{};
				allocInfo.descriptorPool = *descriptorPool,
				allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
				allocInfo.pSetLayouts = layouts.data();

			gameObject.descriptorSets.clear();
			gameObject.descriptorSets = device.allocateDescriptorSets(allocInfo);
			for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

				vk::DescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = *gameObject.uniformBuffers[i],
					bufferInfo.offset = 0,
					bufferInfo.range = sizeof(UniformBufferObject);

				vk::DescriptorImageInfo imageInfo{};
				imageInfo.sampler = *textureSampler,
					imageInfo.imageView = *textureImageView,
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

				device.updateDescriptorSets(descriptorWrites, {});
			}
		}
	}

	void VulkanDevice::createCommandBuffer() {

		commandBuffers.clear();
		vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.commandPool = commandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		commandBuffers = vk::raii::CommandBuffers(device, allocInfo);

		/*vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.commandPool = commandPool, 
			allocInfo.level = vk::CommandBufferLevel::ePrimary, 
			allocInfo.commandBufferCount = 1 ;
		commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());*/
	}

	void VulkanDevice::recordCommandBuffer(uint32_t imageIndex) {

		
		auto& commandBuffer = commandBuffers[frameIndex];
		commandBuffer.begin({});
		transition_image_layout(
			swapChainImages[imageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                        // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
			vk::ImageAspectFlagBits::eColor);
		// Transition the multisampled color image to COLOR_ATTACHMENT_OPTIMAL
		transition_image_layout(
			*colorImage,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor);
		// Transition the depth image to DEPTH_ATTACHMENT_OPTIMAL
		transition_image_layout(
			*depthImage,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

		vk::RenderingAttachmentInfo attachmentInfo = {};
			attachmentInfo.imageView = colorImageView,
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage,
			attachmentInfo.resolveImageView = swapChainImageViews[imageIndex],
			attachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear,
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore,
			attachmentInfo.clearValue = clearColor;


		vk::RenderingAttachmentInfo depthAttachmentInfo{};
			depthAttachmentInfo.imageView		= depthImageView,
			depthAttachmentInfo.imageLayout		= vk::ImageLayout::eDepthAttachmentOptimal,
			depthAttachmentInfo.loadOp			= vk::AttachmentLoadOp::eClear,
			depthAttachmentInfo.storeOp			= vk::AttachmentStoreOp::eDontCare,
			depthAttachmentInfo.clearValue		= clearDepth;


		vk::RenderingInfo renderingInfo = {};
			renderingInfo.renderArea = {.offset = { 0, 0 }, .extent = swapChainExtent },
			renderingInfo.layerCount = 1,
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachments = &attachmentInfo,
			renderingInfo.pDepthAttachment = &depthAttachmentInfo;
			
		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

		commandBuffer.bindVertexBuffers(0, *vertexBuffer, { 0 });
	

		commandBuffer.bindIndexBuffer(*indexBuffer,0,vk::IndexType::eUint32);

		for (const auto& gameObject : meshes)
		{
			// Bind the descriptor set for this object
			commandBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*pipelineLayout,
				0,
				*gameObject.descriptorSets[frameIndex],
				nullptr);

			// Draw the object
			commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);
		}


		//commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);
		//commandBuffer.drawIndexed(indices.size(), 1 ,0, 0, 0); //one 3 btw
		////commandBuffer.drawIndexed(12, 1, 0, 0, 0); //one 3 btw

		commandBuffer.endRendering();
		transition_image_layout(
			swapChainImages[imageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,              // dstStage
			vk::ImageAspectFlagBits::eColor
		);
		commandBuffer.end();
	}

	void VulkanDevice::transition_image_layout(
		vk::Image       image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask,
		vk::ImageAspectFlags    image_aspect_flags
	) 
	{
		vk::ImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor,
			subresourceRange.baseMipLevel = 0,
			subresourceRange.levelCount = 1,
			subresourceRange.baseArrayLayer = 0,
			subresourceRange.layerCount = 1;
		vk::ImageSubresourceRange range{};
			range.aspectMask = image_aspect_flags,
			range.baseMipLevel = 0,
			range.levelCount = 1,
			range.baseArrayLayer = 0,
			range.layerCount = 1;

		vk::ImageMemoryBarrier2 barrier = {};
		barrier.srcStageMask = srcStageMask,
			barrier.srcAccessMask = srcAccessMask,
			barrier.dstStageMask = dstStageMask,
			barrier.dstAccessMask = dstAccessMask,
			barrier.oldLayout = oldLayout,
			barrier.newLayout = newLayout,
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			barrier.image = image;
			barrier.subresourceRange = range;
            

			vk::DependencyInfo dependencyInfo = {};
			dependencyInfo.dependencyFlags = {},
			dependencyInfo.imageMemoryBarrierCount = 1,
			dependencyInfo.pImageMemoryBarriers = &barrier;

		commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
	}

	void VulkanDevice::drawFrame() {

		//testAnim.interpolate(frameNumber);
		//updateVertexBuffer();
		updateUniformBuffer(frameIndex);
		auto fenceResult = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			throw std::runtime_error("failed to wait for fence!");
		}
		auto [result, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);

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
		device.resetFences(*inFlightFences[frameIndex]);
		commandBuffers[frameIndex].reset();
		recordCommandBuffer(imageIndex);
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo submitInfo{
										1,
										&* presentCompleteSemaphores[frameIndex],
										& waitDestinationStageMask,
										1,
										&* commandBuffers[frameIndex],
										1,
										&*renderFinishedSemaphores[imageIndex]};

		queue.submit(submitInfo, *inFlightFences[frameIndex]);
																			//bruhhhhhhh
		const vk::PresentInfoKHR presentInfoKHR{1, &*renderFinishedSemaphores[imageIndex],1, &*swapChain,&imageIndex};

		if (framebufferResized)
		{
			framebufferResized = false;
			recreateSwapChain();
			std::cout << "resize" << "\n";
			return;
		}


		result = queue.presentKHR(presentInfoKHR); //error here 
		
			switch (result)
		{
		case vk::Result::eSuccess:
			break;
		case vk::Result::eSuboptimalKHR:
			framebufferResized = false;
			recreateSwapChain();
			std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
			break;
		default:
			framebufferResized = false;
			recreateSwapChain();
			std::cerr << "what the fuck" << "\n";
			break;        // an unexpected result is returned!
		}
			frameNumber++;
		frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void VulkanDevice::createSyncObjects() {


		assert(presentCompleteSemaphores.empty() && renderFinishedSemaphores.empty() && inFlightFences.empty());

		for (size_t i = 0; i < swapChainImages.size(); i++)
		{
			renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{	
			vk::FenceCreateInfo info{};
			info.flags = vk::FenceCreateFlagBits::eSignaled;
			presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
			inFlightFences.emplace_back(device, info);
		}
		/*presentCompleteSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
		renderFinishedSemaphore = vk::raii::Semaphore(device, vk::SemaphoreCreateInfo());
		vk::FenceCreateInfo info{};
		info.flags = vk::FenceCreateFlagBits::eSignaled;
		drawFence = vk::raii::Fence(device, info);*/

	}

	void VulkanDevice::Exit() {

		//device.waitIdle();
		//error during cleanup
		cleanupSwapChain();

	}

	void VulkanDevice::cleanupSwapChain()
	{
		swapChainImageViews.clear();
		swapChain = nullptr;
	}

	void VulkanDevice::recreateSwapChain() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(window.getGLFWWindow(), &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window.getGLFWWindow(), &width, &height);
			glfwWaitEvents();
		}
		device.waitIdle();

		cleanupSwapChain();


		createSwapChain();
		createImageViews();
		createColorResources();
		createDepthResources();
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

	void VulkanDevice::createDepthResources() {
		vk::Format depthFormat = findDepthFormat();
		createImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
		//createImage(swapChainExtent.width, swapChainExtent.height, 1, vk::SampleCountFlagBits::e1, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
		depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1);
	}
	vk::Format VulkanDevice::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) {
		
		
		auto formatIt = std::ranges::find_if(candidates, [&](auto const format) {
			vk::FormatProperties props = physicalDevice.getFormatProperties(format);
			return (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
				((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)));
			});
		if (formatIt == candidates.end())
		{
			throw std::runtime_error("failed to find supported format!");
		}
		return *formatIt;
	}
	bool VulkanDevice::hasStencilComponent(vk::Format format) {

		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}
	vk::Format VulkanDevice::findDepthFormat() {
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment
		);
	}

	vk::SampleCountFlagBits VulkanDevice::getMaxUsableSampleCount() {
		vk::PhysicalDeviceProperties physicalDeviceProperties = physicalDevice.getProperties();

		vk::SampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & vk::SampleCountFlagBits::e64) { return vk::SampleCountFlagBits::e64; }
		if (counts & vk::SampleCountFlagBits::e32) { return vk::SampleCountFlagBits::e32; }
		if (counts & vk::SampleCountFlagBits::e16) { return vk::SampleCountFlagBits::e16; }
		if (counts & vk::SampleCountFlagBits::e8) { return vk::SampleCountFlagBits::e8; }
		if (counts & vk::SampleCountFlagBits::e4) { return vk::SampleCountFlagBits::e4; }
		if (counts & vk::SampleCountFlagBits::e2) { return vk::SampleCountFlagBits::e2; }

		return vk::SampleCountFlagBits::e1;
	}
	
	void VulkanDevice::setupMeshes()
	{
		meshes[0].position = { 0.0f, 0.0f, 0.0f };
		meshes[0].rotation = { 0.0f, 0.0f, 0.0f };
		meshes[0].scale = { 1.0f, 1.0f, 1.0f };

		// Object 2 - Left
		meshes[1].position = { -2.0f, 0.0f, -1.0f };
		meshes[1].rotation = { 0.0f, glm::radians(45.0f), 0.0f };
		meshes[1].scale = { 0.75f, 0.75f, 0.75f };

		// Object 3 - Right
		meshes[2].position = { 2.0f, 0.0f, -1.0f };
		meshes[2].rotation = { 0.0f, glm::radians(-45.0f), 0.0f };
		meshes[2].scale = { 0.75f, 0.75f, 0.75f };
	}

	void VulkanDevice::prepareModels() {
		for (int i = 0; i < models.size(); i++) {
			vertices.emplace_back(std::vector<Vertex>());
			indices.emplace_back(std::vector<uint32_t>());
		}
	}
}
