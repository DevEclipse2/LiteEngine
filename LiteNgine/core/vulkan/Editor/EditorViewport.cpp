#include "EditorViewport.h"
#include <cmath>
#include "../InterfaceLayers/Lt_ILayer.h"
#include "../EngineClasses/Lt_Importer.h"
#include "../EngineClasses/rendering/RenderData.h"
#include <numbers>
namespace lte {
	uint8_t deviceID = 0;
	void EditorViewport::Init(ImVec2 Size, uint8_t FramesInFlight)
	{
		auto& deviceSet = Lt_Vulkan::devices[deviceID];
		framesInFlight = FramesInFlight;
		size = Size;
		createImages();
		Con::Log("create desc set layoud", TAG_ENGINE);
		PipelineDelegate::createDescriptorSetLayout(pipeline.descSetLayout, deviceSet.logicalDevice);
		Con::Log("create pipeline", TAG_ENGINE);

		createPipeline();
		Con::Log("create synchronization", TAG_ENGINE);

		LtSync::createSyncObjects(syncSet, framesInFlight, &Lt_Vulkan::devices[0].logicalDevice, framesInFlight);
		auto& physDev = Lt_Vulkan::devices[0].physicalDevice;
		Con::Log("create command buffer", TAG_ENGINE);

		CommandBuffers::createCommandBuffer(&commandBuffers, &Lt_Vulkan::commandPool, &Lt_Vulkan::devices[0].logicalDevice, framesInFlight);
		singleTimeCommandInfo info{&Lt_Vulkan::devices[0].logicalDevice, &Lt_Vulkan::commandPool ,&Lt_Vulkan::devices[0].queue};
		Con::Log("load files", TAG_ENGINE);


		Lt_Importer::Load("models/skeletalTest.fbx", 0);
		/*FileLoader::TemporaryFileLoad(Lt_Vulkan::devices[0].logicalDevice, physDev,info);
		*/
		Lt_Importer::GenerateRenderSets(info, deviceSet.physicalDevice);
		renderSets = Lt_Importer::renderSets;
		Lt_Importer::RemoveModels();

		Con::Log("create sampler", TAG_ENGINE);
		DeviceHandler::createTextureSampler(&sampler, physDev, Lt_Vulkan::devices[0].logicalDevice);
		
		/*Con::Log("create vtx buffer", TAG_ENGINE);
		Buffers::createVertexBuffer(FileLoader::VertexesSize, FileLoader::VertexArray, &vertexBuffer, &vertexBufferMemory, info, physDev);
		Con::Log("create idx buffer", TAG_ENGINE);

		Buffers::createIndexBuffer(FileLoader::IndicesSize, FileLoader::IndicesArray, &indexBuffer, &indexBufferMemory, info, physDev);*/
		//fix this later

		meshes.push_back(LtMeshInfo{});
		meshes.push_back(LtMeshInfo{});
		//MeshInfo.push_back(LtMeshInfo{});
		meshes[0].position = { 0.0f, 0.0f, -1.0f };
		meshes[0].rotation = { 0.0f, 0.0f, 0.0f };
		meshes[0].scale = { 1.1f, 1.1f,1.1f };

		meshes[1].position = { -2.0f, 0.0f, -1.0f };
		meshes[1].rotation = { glm::radians(90.0f) ,0.0f , 0.0f};
		meshes[1].scale = { 0.45f, 0.45f, 0.45f };

		Buffers::createUniformBuffers(&meshes, framesInFlight, Lt_Vulkan::devices[0].logicalDevice, physDev);
		DeviceHandler::createDescriptorPool(&descriptorPool, &Lt_Vulkan::devices[0].logicalDevice, 2, framesInFlight);
		DeviceHandler::createDescriptorSets(pipeline.descSetLayout,descriptorPool,sampler,meshes,framesInFlight,deviceSet.logicalDevice,renderSets);

		descriptorSets.resize(framesInFlight);
		for (int i = 0; i < FramesInFlight; i++) 
		{
			descriptorSets[i] = ImGui_ImplVulkan_AddTexture(*images[i]->imageSampler, *images[i]->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		//load models, create descriptor sets and rendersets and stuff
	}
	void EditorViewport::Recreate(ImVec2 Size, uint8_t FramesInFlight) {
		Con::LogEvent("recreating scene Viewport with dimensions :" + std::to_string(Size.x) + "," + std::to_string(Size.y) + "with " + std::to_string(FramesInFlight) + " frames in flight", TAG_ENGINE | TAG_VULKAN);
		
		bool recreateAll = framesInFlight != FramesInFlight;

		for (int i = 0; i < framesInFlight; i++) {
			ImGui_ImplVulkan_RemoveTexture(descriptorSets[i]);
		}
		for (int i = 0; i < framesInFlight; i++) {
			auto fenceResult = Lt_Vulkan::devices[0].logicalDevice.waitForFences(*syncSet.inFlightFences[swapFrame], vk::True, UINT64_MAX);
			if (fenceResult != vk::Result::eSuccess)
			{
				Con::LogError("failed to wait for fence", CRIT_SEVERITY, TAG_ENGINE | TAG_VULKAN);
				throw std::runtime_error("failed to wait for fence!");
			}
		}
		framesInFlight = FramesInFlight;
		
		size = Size;
		//change sizes, create images again, remove the imgui images
		
		createImages();
		createPipeline();
		if (recreateAll) 
		{
			
			Lt_Vulkan::devices[0].logicalDevice.waitIdle();
			LtSync::createSyncObjects(syncSet, framesInFlight, &Lt_Vulkan::devices[0].logicalDevice, framesInFlight);
			auto& physDev = Lt_Vulkan::devices[0].physicalDevice;
			CommandBuffers::createCommandBuffer(&commandBuffers, &Lt_Vulkan::commandPool, &Lt_Vulkan::devices[0].logicalDevice, framesInFlight);
			singleTimeCommandInfo info{ &Lt_Vulkan::devices[0].logicalDevice, &Lt_Vulkan::commandPool ,&Lt_Vulkan::devices[0].queue };
			Buffers::createUniformBuffers(&meshes, framesInFlight, Lt_Vulkan::devices[0].logicalDevice, physDev);
			DeviceHandler::createDescriptorPool(&descriptorPool, &Lt_Vulkan::devices[0].logicalDevice, 2, framesInFlight);
			DeviceHandler::createDescriptorSets(pipeline.descSetLayout, descriptorPool, sampler, meshes, framesInFlight, Lt_Vulkan::devices[0].logicalDevice, renderSets);
			descriptorSets.resize(framesInFlight);
		}	
		for (int i = 0; i < framesInFlight; i++)
		{
			descriptorSets[i] = ImGui_ImplVulkan_AddTexture(*images[i]->imageSampler, *images[i]->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		
	}
	void EditorViewport::createImages() {

		auto& deviceSet = Lt_Vulkan::devices[deviceID];
		vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e8;
		int deviceID = 0;
		//colorimage
		vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;

		ImageDelegate::createImage(colorImage, size.x, size.y, 1, samples, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(colorImage, colorFormat, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);

		//depth image
		vk::Format depthFormat = PipelineDelegate::findDepthFormat(deviceSet.physicalDevice);
		ImageDelegate::createImage(depthImage, size.x, size.y, 1, samples, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
		ImageDelegate::createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth, 1, deviceSet.logicalDevice);
		
		images.clear();


		//swap images
		for (int i = 0; i < framesInFlight; i++) {
			Con::Log("Making swap image" + std::to_string(i), TAG_ENGINE);
			LtImage swapImg{};
			ImageDelegate::createImage(swapImg, size.x, size.y, 1, vk::SampleCountFlagBits::e1, colorFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, deviceSet.logicalDevice, deviceSet.physicalDevice);
			ImageDelegate::createImageView(swapImg, colorFormat, vk::ImageAspectFlagBits::eColor, 1, deviceSet.logicalDevice);
			ImageDelegate::createSampler(swapImg, deviceSet.logicalDevice);
			images.emplace_back(std::make_unique<LtImage>(std::move(swapImg)));
		}

	}
	void EditorViewport::createPipeline()
	{
		std::string shaderFilepath = "shaders/slang.spv";
		auto& deviceSet = Lt_Vulkan::devices[0];

		vk::PipelineShaderStageCreateInfo vertShaderInfo{};
		vk::PipelineShaderStageCreateInfo fragShaderInfo{};
		vk::raii::ShaderModule module = PipelineDelegate::createShaderModule(PipelineDelegate::readShaderInfo(nullptr, shaderFilepath), deviceSet.logicalDevice);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex,
			vertShaderStageInfo.module = module,
			vertShaderStageInfo.pName = "vertMain";
		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment,
			fragShaderStageInfo.module = module,
			fragShaderStageInfo.pName = "fragMain";
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
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e8,
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
		vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;

		vk::PipelineRenderingCreateInfoKHR pipelineCreateInfo{ };
		pipelineCreateInfo.sType = vk::StructureType::ePipelineRenderingCreateInfoKHR;
		pipelineCreateInfo.pNext = NULL;
		pipelineCreateInfo.viewMask = 0;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.pColorAttachmentFormats = &colorFormat;
		pipelineCreateInfo.depthAttachmentFormat = depthFormat;
		pipelineCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;


		vk::raii::PipelineLayout pipelineLayout(deviceSet.logicalDevice, pipelineLayoutInfo); 

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

		ImageDelegate::transition_image_layout(
			*depthImage.image,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth, commandBuffer);

		vk::ClearValue clearColor = vk::ClearColorValue(0.8f, 0.0f, 0.1f, 0.0f);
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

		commandBuffer.bindVertexBuffers(0, *vertexBuffer, { 0 });
		commandBuffer.bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint32);

		uint64_t objectid = 0;

		for (const auto& gameObject : meshes)
		{
			// Bind the descriptor set for this object
			commandBuffer.bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				*pipeline.PipelineLayout,
				0,
				*gameObject.descriptorSets[swapFrame],
				nullptr);
			// Draw the object
			commandBuffer.drawIndexed(renderSets[objectid].IndiceArraySize, 1, renderSets[objectid].IndiceArrayStartIndex, renderSets[objectid].vertexArrayStartIndex, 0);
			objectid++;
		}
		commandBuffer.endRendering();
		ImageDelegate::transition_image_layout(
			*(images[swapFrame]->image),
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::AccessFlagBits2::eColorAttachmentWrite,
			vk::AccessFlagBits2::eShaderRead,                       // dstAccessMask: We will read this in a shader!
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,
			vk::PipelineStageFlagBits2::eFragmentShader,            // dstStage: The fragment shader (ImGui) is waiting for this!
			vk::ImageAspectFlagBits::eColor,
			commandBuffer
		);
		commandBuffer.end();
	}

	void EditorViewport::UpdateUniformBuffers()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
		fps = 1 / (time - prevtime);
		frameTime = (time - prevtime) * 1000;
		UniformBufferObject ubo{};
		glm::mat4 view = camera.getViewMatrix();
		glm::mat4 proj = glm::perspective(glm::radians(FOV),
			static_cast<float>(size.x) / static_cast<float>(size.y),
			0.1f, 20.0f);

		ubo.proj[1][1] *= -1;
		// Update uniform buffers for each object
		for (auto& gameObject : meshes) {
			// Get the model matrix for this object
			glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::mat4 model = gameObject.getModelMatrix() * initialRotation;
			// Create and update the UBO
			UniformBufferObject ubo{};
			ubo.model = model,
				ubo.view = view,
				ubo.proj = proj;
			// Copy the UBO data to the mapped memory
			memcpy(gameObject.uniformBuffersMapped[swapFrame], &ubo, sizeof(ubo));
		}
		prevtime = time;
	}

	void EditorViewport::SubmitGUICommands() 
	{
		if (!Enabled) 
		{
			return;
		}
		ImGui::Begin("big VP (viewport)", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("resolution"))
			{
				ImGui::Text("unfinished");
				//in built layouts

				if (ImGui::MenuItem("Undo", "CTRL+Z")) { /* load layout here */ }
				//separator
				//save as button
				//revert button
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
			ImGui::SetNextItemWidth(200.0f);
			ImGui::InputInt("width", &newWidth);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200.0f);
			ImGui::InputInt("height", &newHeight);
			ImGui::SliderFloat("scale", &scale,0.1f,10.0f);
			
		if (ImGui::Button("Apply", ImVec2(120, 50)))
		{
				Con::LogEvent("Recreating Viewport", TAG_ENGINE | TAG_VULKAN);
				Recreate(ImVec2(newWidth, newHeight), framesInFlight);
		}
		else
		{
			ImVec2 cursor = ImGui::GetCursorPos();
			ImGui::Image(descriptorSets[swapFrame], ImVec2(size.x * scale, size.y * scale));

			// 2. Handle Inputs Only When Hovering/Interacting With This Specific Window
			ImGuiIO& io = ImGui::GetIO();

			// Check if mouse is hovering over this viewport window
			bool isHovered = ImGui::IsWindowHovered();

			// Right-click initiated inside the viewport
			if (isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				isDragging = true;
				lastMousePos = io.MousePos;
			}

			// Right-click released
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				isDragging = false;
			}

			// Handle mouse movement while dragging
			if (isDragging) {
				// Calculate how much the mouse moved since last frame
				float xOffset = io.MousePos.x - lastMousePos.x;
				float yOffset = lastMousePos.y - io.MousePos.y; // Inverted Y behavior

				lastMousePos = io.MousePos;

				// Feed the delta into your camera
				camera.processMouseMovement(xOffset, yOffset,true);

				// Lock the mouse cursor inside the ImGui window while dragging (Optional but helpful)
				ImGui::SetMouseCursor(ImGuiMouseCursor_None);
			}

			// 3. Handle WASD Keyboard Input while dragging the camera
			if (isDragging) {
				// We use ImGui's key down queries to remain context-aware
				if (ImGui::IsKeyDown(ImGuiKey_W)) camera.processKeyboard(0, frameTime/1000);
				if (ImGui::IsKeyDown(ImGuiKey_S)) camera.processKeyboard(1, frameTime/1000);
				if (ImGui::IsKeyDown(ImGuiKey_A)) camera.processKeyboard(2, frameTime/1000);
				if (ImGui::IsKeyDown(ImGuiKey_D)) camera.processKeyboard(3, frameTime/1000);
			}
			
		}
		

		if (objectID == SelectedObject) 
		{
			meshes[objectID].position.x = x;
			meshes[objectID].position.y = y;
			meshes[objectID].position.z = z;
			meshes[objectID].rotation.x = glm::radians(rotX);
			meshes[objectID].rotation.y = glm::radians(rotY);
			meshes[objectID].rotation.z = glm::radians(rotZ);
		}
		else if (objectID >= 0 && objectID < meshes.size()) {
			x = meshes[objectID].position.x;
			y = meshes[objectID].position.y;
			z = meshes[objectID].position.z;
			rotX = glm::degrees(meshes[objectID].rotation.x);
			rotY = glm::degrees(meshes[objectID].rotation.y);
			rotZ = glm::degrees(meshes[objectID].rotation.z);
			SelectedObject = objectID;
		}
		ImGui::SameLine();
		ImGui::Text(("FPS " + std::to_string(fps) + "\n" + "frameTime " + std::to_string(frameTime)+ '\n' + "frameID " + std::to_string(Lt_ILayer::frameCount) + "\n" + "SwapFrame " + std::to_string(swapFrame)).c_str());
		ImGui::End();
		ImGui::Begin("Object Menu");
		ImGui::InputInt("ObjectNumber", &objectID);
		ImGui::Text("Position");
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("X##1",&x,0.05f);
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("Y##1",&y,0.05f);
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("Z##1",&z,0.05f);
		ImGui::Text("Rotation");
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("X##2",&rotX);
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("Y##2",&rotY);
		ImGui::SameLine(); ImGui::SetNextItemWidth(100); ImGui::DragFloat("Z##2",&rotZ);
		ImGui::End();



	}
	
	void EditorViewport::RenderScene(vk::raii::Semaphore& signalSemaphore)
	{
		
		auto& cmdBuf = commandBuffers[swapFrame];
		uint64_t handleValue = (uint64_t)(VkCommandBuffer)*cmdBuf;
		UpdateUniformBuffers();

		auto fenceResult = Lt_Vulkan::devices[0].logicalDevice.waitForFences(*syncSet.inFlightFences[swapFrame], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess)
		{
			Con::LogError("failed to wait for fence", CRIT_SEVERITY, TAG_ENGINE | TAG_VULKAN);
			throw std::runtime_error("failed to wait for fence!");
		}
		Lt_Vulkan::devices[0].logicalDevice.resetFences(*syncSet.inFlightFences[swapFrame]);

		cmdBuf.reset();
		SubmitCommands(cmdBuf);
		/*int frame = swapFrame - 1;
		if (frame < 0) {
			frame = framesInFlight + swapFrame;
		}*/
		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		const vk::SubmitInfo submitInfo{
			1,
			//here
			&*signalSemaphore,
			&waitDestinationStageMask,
			1,
			&*commandBuffers[swapFrame],
			1,
			&*syncSet.renderFinishedSemaphores[swapFrame]};
		Lt_Vulkan::devices[0].queue.submit(submitInfo, *syncSet.inFlightFences[swapFrame]);
	}
	void EditorViewport::FinishFrame()
	{
		swapFrame++;
		swapFrame %= framesInFlight;
	}
	EditorViewport::EditorViewport()
	{
	}
	EditorViewport::~EditorViewport()
	{
		for (int i = 0; i < framesInFlight; i++) {
			ImGui_ImplVulkan_RemoveTexture(descriptorSets[i]);
		}
	}
}