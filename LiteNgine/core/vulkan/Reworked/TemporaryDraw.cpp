#include "TemporaryDraw.h"
namespace lte {

	void TemporaryDraw::updateUniformBuffer(uint32_t frame , LtSwapChain* swap, std::vector<LtMeshInfo>* info)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		fps = 1 / (time - prevtime);
		frameTime = (time - prevtime) * 1000;
		UniformBufferObject ubo{};

		glm::mat4 view = glm::lookAt(glm::vec3(2.0f, -6.0f, 6.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f),
			static_cast<float>(swap->swapChainExtent.width) / static_cast<float>(swap->swapChainExtent.height),
			0.1f, 20.0f);

		ubo.proj[1][1] *= -1;
		// Update uniform buffers for each object
		for (auto& gameObject : *info) {
			// Apply continuous rotation to the object
			const float rotationSpeed = 0.5f;                          // Rotation speed in radians per second
			gameObject.rotation.y += rotationSpeed * (time - prevtime);

			// Get the model matrix for this object
			glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 model = gameObject.getModelMatrix() * initialRotation;

			// Create and update the UBO
			UniformBufferObject ubo{};
			ubo.model = model,
				ubo.view = view,
				ubo.proj = proj;


			// Copy the UBO data to the mapped memory
			memcpy(gameObject.uniformBuffersMapped[frame], &ubo, sizeof(ubo));
		}
		prevtime = time;
	}
	void TemporaryDraw::recordCommandBuffer(uint32_t imageIndex, uint32_t frameIndex,vk::raii::CommandBuffer* commandBuffer,LtSwapChain* swapChainImage,
		LtImage* colorImage, LtImage* depthImage, LtPipeline* pipeline,vk::raii::Buffer* vertexBuf,vk::raii::Buffer* indexBuf,
		vk::raii::DeviceMemory* vertexMem,vk::raii::DeviceMemory* indiceMem, std::vector<LtMeshInfo>* meshes, std::vector<RenderSet>* rendersets)
	{


		commandBuffer->begin({});
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
			*colorImage->image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::ImageAspectFlagBits::eColor, commandBuffer);
		// Transition the depth image to DEPTH_ATTACHMENT_OPTIMAL
		ImageDelegate::transition_image_layout(
			*depthImage->image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth,commandBuffer);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.05f, 0.1f, 1.0f);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

		vk::RenderingAttachmentInfo attachmentInfo = {};
			attachmentInfo.imageView = colorImage->imageView,
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage,
			attachmentInfo.resolveImageView = swapChainImage->imageViews[imageIndex],
			attachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eClear,
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore,
			attachmentInfo.clearValue = clearColor;


		vk::RenderingAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.imageView = depthImage->imageView,
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

		commandBuffer->beginRendering(renderingInfo);
		commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->pipeline);
		commandBuffer->setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainImage->swapChainExtent.width), static_cast<float>(swapChainImage->swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer->setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainImage->swapChainExtent));

		commandBuffer->bindVertexBuffers(0, **vertexBuf, { 0 });
		commandBuffer->bindIndexBuffer(**indexBuf, 0, vk::IndexType::eUint32);

		uint64_t objectid = 0;
		uint64_t vertexOffsets = 0;
		uint64_t indexOffsets = 0;
		for (const auto& gameObject : *meshes)
		{
			// Bind the descriptor set for this object
			commandBuffer->bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*pipeline->PipelineLayout,
				0,
				*gameObject.descriptorSets[frameIndex],
				nullptr);

			// Draw the object

			commandBuffer->drawIndexed(rendersets->at(objectid).IndiceArraySize, 1, indexOffsets, vertexOffsets, 0);
			vertexOffsets += rendersets->at(objectid).vertexArraySize;
			indexOffsets  += rendersets->at(objectid).IndiceArraySize;
			objectid++;
		}
		commandBuffer->endRendering();
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
		commandBuffer->end();
	}

}
