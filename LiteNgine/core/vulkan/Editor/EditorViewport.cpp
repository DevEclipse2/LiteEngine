#include "EditorViewport.h"
namespace lte {
	auto& deviceSet = Lt_Vulkan::devices[deviceID];



	
	void EditorViewport::Init(ImVec2 Size, uint8_t FramesInFlight)
	{
		framesInFlight = FramesInFlight;
		size = Size;
		createImages();
		createPipeline();
		inline auto& physDev = Lt_Vulkan::devices[0].physicalDevice;
		CommandBuffers::createCommandBuffer(&commandBuffers, &Lt_Vulkan::commandPool, &Lt_Vulkan::devices[0].logicalDevice, framesInFlight);
		singleTimeCommandInfo info{&Lt_Vulkan::devices[0].logicalDevice, &Lt_Vulkan::commandPool ,&Lt_Vulkan::devices[0].queue};
		FileLoader::TemporaryFileLoad(Lt_Vulkan::devices[0].logicalDevice, physDev,info);
		renderSets = FileLoader::renderSets;


		DeviceHandler::createTextureSampler(&sampler, physDev, Lt_Vulkan::devices[0].logicalDevice);

		Buffers::createVertexBuffer(FileLoader::VertexesSize, FileLoader::VertexArray, &vertexBuffer, &vertexBufferMemory, info, physDev);
		Buffers::createIndexBuffer(FileLoader::IndicesSize, FileLoader::IndicesArray, &indexBuffer, &indexBufferMemory, info, physDev);
		//fix this later

		MeshInfo.push_back(LtMeshInfo{});
		MeshInfo.push_back(LtMeshInfo{});
		//MeshInfo.push_back(LtMeshInfo{});
		MeshInfo[0].position = { 0.0f, 0.0f, -1.0f };
		MeshInfo[0].rotation = { glm::radians(90.0f), 0.0f, 0.0f };
		MeshInfo[0].scale = { 1.1f, 1.1f,1.1f };

		MeshInfo[1].position = { -2.0f, 0.0f, -1.0f };
		MeshInfo[1].rotation = { 0.0f, 0.0f, 0.0f };
		MeshInfo[1].scale = { 0.45f, 0.45f, 0.45f };

		Buffers::createUniformBuffers(&MeshInfo, framesInFlight, Lt_Vulkan::devices[0].logicalDevice, physDev);
		DeviceHandler::createDescriptorPool(&pool, &Lt_Vulkan::devices[0].logicalDevice, 2, framesInFlight);

		//load models, create descriptor sets and rendersets and stuff
	}
	void EditorViewport::Recreate(ImVec2 Size, uint8_t FramesInFlight) {
		framesInFlight = FramesInFlight;
		size = Size;
		createImages();
		createPipeline();
	}
	void EditorViewport::createImages() {

		vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
		int deviceID = 0;
		//colorimage
		vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;
		ImageDelegate::createImage(colorImage, size.x, size.y, 1, samples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);

		//depth image
		vk::Format depthFormat = PipelineDelegate::findDepthFormat(deviceSet.physicalDevice);
		ImageDelegate::createImage(depthImage, size.x, size.y, 1, samples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1, deviceSet.logicalDevice);
		images.clear();


		//swap images
		for (int i = 0; i < framesInFlight; i++) {
			LtImage swapImg{};
			ImageDelegate::createImage(swapImg, size.x, size.y, 1, samples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
			ImageDelegate::createImageView(swapImg, colorFormat, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);
			images.emplace_back(std::move(swapImg));
		}

	}
	void EditorViewport::createPipeline()
	{
		std::string shaderFilepath = "shaders/shader.slang";
		std::string vertShadername = "vertex";
		std::string fragShadername = "fragment";


		PipelineDelegate::createDescriptorSetLayout(pipeline.descSetLayout, deviceSet.logicalDevice);


		vk::PipelineShaderStageCreateInfo vertShaderInfo{};
		vk::PipelineShaderStageCreateInfo fragShaderInfo{};
		vk::raii::ShaderModule module = PipelineDelegate::createShaderModule(PipelineDelegate::readShaderInfo(nullptr, shaderFilepath), deviceSet.logicalDevice);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex,
			vertShaderStageInfo.module = module,
			vertShaderStageInfo.pName = vertShadername.c_str();
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment,
			fragShaderStageInfo.module = module,
			fragShaderStageInfo.pName = fragShadername.c_str();
		//defines pipeline

		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.vertexBindingDescriptionCount = 1,
			vertexInputInfo.pVertexBindingDescriptions = &bindingDescription,
			vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
			vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
		vk::PipelineViewportStateCreateInfo      viewportState{};
		viewportState.viewportCount = 1, viewportState.scissorCount = 1;


		//vk::PipelineRasterizationStateCreateInfo rasterizer{};
		vk::PipelineRasterizationStateCreateInfo rasterizer({}, vk::False, vk::False, vk::PolygonMode::eFill,
			vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, vk::False, 0.0f, 0.0f, 1.0f, 1.0f);
		vk::PipelineMultisampleStateCreateInfo multisampling{};
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e16,
			multisampling.sampleShadingEnable = vk::False;
		vk::PipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.depthTestEnable = vk::True,
			depthStencil.depthWriteEnable = vk::True,
			depthStencil.depthCompareOp = vk::CompareOp::eLess,
			depthStencil.depthBoundsTestEnable = vk::False,
			depthStencil.stencilTestEnable = vk::False;
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
		pipelineLayoutInfo.setLayoutCount = 1,
			pipelineLayoutInfo.pSetLayouts = &*pipeline.descSetLayout,
			pipelineLayoutInfo.pushConstantRangeCount = 0;

		vk::Format depthFormat = PipelineDelegate::findDepthFormat(Lt_Vulkan::devices[deviceID].physicalDevice);

		vk::PipelineRenderingCreateInfoKHR pipelineCreateInfo{ };
		pipelineCreateInfo.sType = vk::StructureType::ePipelineRenderingCreateInfoKHR;
		pipelineCreateInfo.pNext = NULL;
		pipelineCreateInfo.viewMask = 0;
		pipelineCreateInfo.colorAttachmentCount = 1;
		//pipelineCreateInfo.pColorAttachmentFormats = ;
		pipelineCreateInfo.depthAttachmentFormat = depthFormat;
		pipelineCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;

		vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;

		vk::raii::PipelineLayout pipelineLayout = vk::raii::PipelineLayout(deviceSet.logicalDevice, pipelineLayoutInfo);


		pipeline.createPipeline(std::size(shaderStages), shaderStages, &vertexInputInfo, &inputAssembly, &viewportState, &rasterizer, &multisampling, &colorBlending, &dynamicState, &depthStencil, pipelineLayout, 1,
			&colorFormat, //colorformat
			depthFormat, deviceSet.logicalDevice);
	}

	void EditorViewport::SubmitCommands(vk::raii::CommandBuffer& commandBuffer)
	{
		commandBuffer.begin({});
		ImageDelegate::transition_image_layout(
			*(images[swapFrame]->image),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                        // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // dstStage
			vk::ImageAspectFlagBits::eColor, commandBuffer);
		// Transition the multisampled color image to COLOR_ATTACHMENT_OPTIMAL
		ImageDelegate::transition_image_layout(
			*colorImage.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor, commandBuffer);
		// Transition the depth image to DEPTH_ATTACHMENT_OPTIMAL

		ImageDelegate::transition_image_layout(
			*depthImage.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth, commandBuffer);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.05f, 0.1f, 1.0f);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

		vk::RenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.imageView = *colorImage.imageView,
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage,
			attachmentInfo.resolveImageView = *(images[swapFrame]->imageView),
			attachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear,
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore,
			attachmentInfo.clearValue = clearColor;

		vk::RenderingAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.imageView = *depthImage.imageView,
			depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eClear,
			depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eDontCare,
			depthAttachmentInfo.clearValue = clearDepth;
			
		VkExtent2D extent = VkExtent2D{ static_cast<unsigned int>(size.x),static_cast<unsigned int>(size.y) };
			
		vk::RenderingInfo renderingInfo = {};
			renderingInfo.renderArea.offset = VkOffset2D{ 0,0 },
			renderingInfo.renderArea.extent = extent,
			renderingInfo.layerCount = 1,
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachments = &attachmentInfo,
			renderingInfo.pDepthAttachment = &depthAttachmentInfo;

		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline.pipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, size.x, size.y, 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

		commandBuffer.bindVertexBuffers(0, **vertexBuf, { 0 });
		commandBuffer.bindIndexBuffer(**indexBuf, 0, vk::IndexType::eUint32);

		uint64_t objectid = 0;

		for (const auto& gameObject : *meshes)
		{
			// Bind the descriptor set for this object
			commandBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*pipeline.PipelineLayout,
				0,
				*gameObject.descriptorSets[swapFrame],
				nullptr);
			// Draw the object
			commandBuffer.drawIndexed(rendersets->at(objectid).IndiceArraySize, 1, rendersets->at(objectid).IndiceArrayStartIndex, rendersets->at(objectid).vertexArrayStartIndex, 0);
			objectid++;
		}
		commandBuffer.endRendering();
		ImageDelegate::transition_image_layout(
			*(images[swapFrame]->image),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,              // dstStage
			vk::ImageAspectFlagBits::eColor,
			commandBuffer
		);
		commandBuffer.end();
	}

	void EditorViewport::UpdateGui()
	{
		frameNum++;
		auto& cmdBuf = commandBuffers[swapFrame];
		UpdateUniformBuffers();
		SubmitCommands(cmdBuf);
		swapFrame++;
	}
}