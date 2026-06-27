#include "EditorViewport.h"
namespace lte {
	auto& deviceSet = Lt_Vulkan::devices[deviceID];



	void EditorViewport::Init() 
	{

		//create image
		//create swapchain
		//create swapchain image views
		int deviceID = 0;
		ImVec2 size = ImVec2(800, 600);
		LtImage Img0;
		auto& deviceSet = Lt_Vulkan::devices[deviceID];

		ImageDelegate::createImage(Img0,size.x, size.y, 1, vk::SampleCountFlagBits::e16, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,vk::MemoryPropertyFlagBits::eDeviceLocal,deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(Img0, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);
		LtImage Img1;
		ImageDelegate::createImage(Img1, size.x, size.y, 1, vk::SampleCountFlagBits::e16, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(Img1, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);

		

		//2 command buffers for each frame

		commandBuffer.begin({});
		ImageDelegate::transition_image_layout(
			swapChainImage->swapChainImages[imageIndex],
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
			attachmentInfo.resolveImageView = *swapChainImage->imageViews[imageIndex],
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


		vk::RenderingInfo renderingInfo = {};
		renderingInfo.renderArea = { .offset = { 0, 0 }, .extent = swapChainImage->swapChainExtent },
			renderingInfo.layerCount = 1,
			renderingInfo.colorAttachmentCount = 1,
			renderingInfo.pColorAttachments = &attachmentInfo,
			renderingInfo.pDepthAttachment = &depthAttachmentInfo;

		commandBuffer.beginRendering(renderingInfo);
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->pipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainImage->swapChainExtent.width), static_cast<float>(swapChainImage->swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainImage->swapChainExtent));

		commandBuffer.bindVertexBuffers(0, **vertexBuf, { 0 });
		commandBuffer.bindIndexBuffer(**indexBuf, 0, vk::IndexType::eUint32);

		uint64_t objectid = 0;

		for (const auto& gameObject : *meshes)
		{
			// Bind the descriptor set for this object
			commandBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*pipeline->PipelineLayout,
				0,
				*gameObject.descriptorSets[frameIndex],
				nullptr);

			// Draw the object

			commandBuffer.drawIndexed(rendersets->at(objectid).IndiceArraySize, 1, rendersets->at(objectid).IndiceArrayStartIndex, rendersets->at(objectid).vertexArrayStartIndex, 0);
			//commandBuffer.drawIndexed(rendersets->at(objectid).IndiceArraySize, 1, rendersets->at(objectid).IndiceArrayStartIndex, rendersets->at(objectid).vertexArrayStartIndex, 0);
			/*vertexOffsets += rendersets->at(objectid).vertexArraySize;
			indexOffsets  += rendersets->at(objectid).IndiceArraySize;*/
			objectid++;
		}
		commandBuffer.endRendering();
		ImageDelegate::transition_image_layout(
			swapChainImage->swapChainImages[imageIndex],
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


	void EditorViewport::Init(ImVec2 Size, uint8_t FramesInFlight)
	{
		framesInFlight = FramesInFlight;
		size = Size;
		createImages();

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
		images->clear();


		//swap images
		for (int i = 0; i < framesInFlight; i++) {
			LtImage swapImg{};
			ImageDelegate::createImage(swapImg, size.x, size.y, 1, samples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
			ImageDelegate::createImageView(swapImg, colorFormat, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);
			images->emplace_back(std::move(swapImg));
		}
		


	}
	void EditorViewport::createPipeline()
	{
		std::string shaderFilepath = "shaders/shader.slang";
		std::string vertShadername = "vertex";
		std::string fragShadername = "fragment";

		LtPipeline pipeline;

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
}