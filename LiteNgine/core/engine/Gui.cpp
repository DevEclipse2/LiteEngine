#include "Gui.h"

void CheckVKResult(VkResult err) {
	if (err == 0) {
		return;
	}
	std::cout << stderr << "[vulkan] error : VkResult = %d\n" << err;
	if (err < 0) {
		abort();
	}
}

namespace lte{
	Gui::Gui(VulkanDevice* vulkDev) : pDevice{ vulkDev }
	{
		getPtrs();
		
		createDescriptorPool();
		createImages();
		InitGUI();
		
		initResources();
		updateBuffers();
		/*
		vk::BufferCreateInfo vertexInfo({}, 1 ,vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		vertexBuffer = device.createBuffer(vertexInfo);

		vk::BufferCreateInfo indexInfo({}, 1, vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		indexBuffer = device.createBuffer(indexInfo);
		// Set up dynamic rendering info
		renderingInfo.colorAttachmentCount = 1;
		vk::Format formats[] = { colorFormat };
		renderingInfo.pColorAttachmentFormats = &colorFormat;*/
	}
	Gui::~Gui(){
		// Wait for device to finish operations before destroying resources
		// NOTE: waitIdle() is acceptable in destructors/cleanup code but should NEVER be used
		// in the main rendering loop as it causes severe performance issues. For frame
		// synchronization, use fences and semaphores instead.
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		if (device) {
			device->waitIdle();
		}

		// All resources are automatically cleaned up by their destructors
		// No manual cleanup needed

		// ImGui context is destroyed separately
	}

	void Gui::getPtrs() {
		
		pFrameIndex = pDevice->getpFrameIndex();
		pipeline = pDevice->getPipeline();
		device = pDevice->getDevice();
		pDevice->getFrameBufferSize(&fbWidth, &fbHeight);
		pColorImage = pDevice->getpColorImage();
		pColorImageMemory = pDevice->getpColorImageMemory();
		pColorImageView = pDevice->getpColorImageView();
		pDepthImageView = pDevice->getDepthImageView();
		depthFormat = pDevice->getDepthFormat();
		swapChainImageViews = pDevice->getSwapChainImageViews();
		
	}
	std::vector<vk::raii::CommandBuffer>* Gui::getpCommandBuffers() {
		return &commandBuffers;
	}
	void Gui::updateFrameBuffer() {

		pDevice->getFrameBufferSize(&fbWidth, &fbHeight);
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize.x = (float)fbWidth;
		io.DisplaySize.y = (float)fbHeight;
	}
	bool Gui::drawFrame()
	{

		ImGui_ImplVulkan_NewFrame();
		ImGui::NewFrame();
		// In your main loop, after ImGui::NewFrame()
		ImGui::SetNextWindowBgAlpha(0.0f); // 0.0f = fully transparent, 1.0f = fully opaque
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		/*	ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);*/
		// Create your UI elements here
		// For example:
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground;
		ImGui::Begin("scene viewport",NULL,window_flags);
		ImGui::Text("Hello, Vulkan!");
		if (ImGui::Button("Click me!")) {
			// Handle button click
			std::cout << "srjitndkf\n";
		}
		ImGui::End();

		ImGui::Begin("Performance: ", &showWindow2);
		pDevice->getProfilingData(&fps, &Frametime, &verticeCount, &indiceCount, &modelCount);
		std::string fpsStr = "Fps :" + std::to_string(fps);
		ImGui::Text(fpsStr.c_str());
		std::string str = "FrameTime : " + std::to_string(Frametime) + " miliseconds";
		ImGui::Text(str.c_str());
		str = "Vertices :" + std::to_string(verticeCount);
		ImGui::Text(str.c_str());
		str = "Indices :" + std::to_string(indiceCount);
		ImGui::Text(str.c_str());
		str = "models :" + std::to_string(modelCount);
		ImGui::Text(str.c_str());
		if (ImGui::Button("Close")) 
			showProfiler = false;
		ImGui::End();

		ImGui::EndFrame();
		ImGui::UpdatePlatformWindows();
		// Render to generate draw data
		ImGui::Render();
		firstFrame = false;

		ImDrawData* drawData = ImGui::GetDrawData();
		if (!drawData || drawData->CmdListsCount == 0) {
			return true;
		}

		recordCommandBuffer(drawData , *pFrameIndex);
		
		
		if (drawData && drawData->CmdListsCount > 0) {
			if (drawData->TotalVtxCount > vertexCount || drawData->TotalIdxCount > indexCount) {
				needsUpdateBuffers = true;
				return true;
			}
		}
		return false;
	}
	void Gui::doDynamicRendering(vk::raii::CommandBuffer &commandBuffer , ImDrawData* data) 
	{

		// Begin dynamic rendering
		vk::RenderingAttachmentInfo attachmentInfo = {};
		attachmentInfo.sType = vk::StructureType::eRenderingAttachmentInfo;
		attachmentInfo.pNext = NULL;
		attachmentInfo.imageView = *pColorImageView,
			attachmentInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.resolveMode = vk::ResolveModeFlagBits::eAverage,
			attachmentInfo.resolveImageView = swapChainImageViews->at(*pFrameIndex),
			attachmentInfo.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			attachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad,
			attachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
			//attachmentInfo.clearValue = clearColor;
		// Note: In a real implementation, you would set imageView, imageLayout,
		// loadOp, storeOp, and clearValue based on your swapchain image

		vk::RenderingAttachmentInfo depthAttachmentInfo{};
		depthAttachmentInfo.imageView = *pDepthImageView,
			depthAttachmentInfo.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			depthAttachmentInfo.loadOp = vk::AttachmentLoadOp::eLoad,
			depthAttachmentInfo.storeOp = vk::AttachmentStoreOp::eStore;
			//depthAttachmentInfo.clearValue = clearDepth;

		vk::RenderingInfo renderingInfo{};
			renderingInfo.renderArea = vk::Rect2D{ {0, 0}, {static_cast<uint32_t>(data->DisplaySize.x),
														   static_cast<uint32_t>(data->DisplaySize.y)} };
			renderingInfo.layerCount = 1;
			renderingInfo.colorAttachmentCount = 1;
			renderingInfo.pColorAttachments = &attachmentInfo;
		// Configure viewport for UI pixel coordinates
		vk::Viewport viewport{};
		viewport.width = data->DisplaySize.x;
		viewport.height = data->DisplaySize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);
		commandBuffer.setViewport(0, viewport);
		commandBuffer.beginRendering(renderingInfo);
		// Bind the pipeline used for ImGui
		
		

	}
	void Gui::recordCommandBuffer(ImDrawData* data, uint8_t index)
	{
		auto& commandBuffer = commandBuffers[index];
		//commandBuffer.reset();
		vk::CommandBufferBeginInfo info{};
		info.sType = vk::StructureType::eCommandBufferBeginInfo;
		info.pNext = NULL;
		info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		info.pInheritanceInfo = NULL;
		commandBuffer.begin(info);
		doDynamicRendering(commandBuffer , data);
		

		//// Convert from ImGui coordinates into NDC via a simple scale/translate
		//pushConstBlock.scale = glm::vec2(2.0f / data->DisplaySize.x, 2.0f / data->DisplaySize.y);
		//pushConstBlock.translate = glm::vec2(-1.0f);
		//commandBuffer.pushConstants<PushConstBlock>(*pipelineLayout, vk::ShaderStageFlagBits::eVertex,
		//	0,pushConstBlock);

		//// We already filled these buffers this frame
		//vk::Buffer vertexBuffers[] = { vertexBuffer };
		//vk::DeviceSize offsets[] = { 0 };
		//commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
		//commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint16);

		//int vertexOffset = 0;
		//int indexOffset = 0;

		//for (int i = 0; i < data->CmdListsCount; i++) {
		//	const ImDrawList* cmdList = data->CmdLists[i];

		//	for (int j = 0; j < cmdList->CmdBuffer.Size; j++) {
		//		const ImDrawCmd* pcmd = &cmdList->CmdBuffer[j];

		//		// Clip per draw call
		//		vk::Rect2D scissor{};
		//		scissor.offset.x = std::max(static_cast<int32_t>(pcmd->ClipRect.x), 0);
		//		scissor.offset.y = std::max(static_cast<int32_t>(pcmd->ClipRect.y), 0);
		//		scissor.extent.width = static_cast<uint32_t>(pcmd->ClipRect.z - pcmd->ClipRect.x);
		//		scissor.extent.height = static_cast<uint32_t>(pcmd->ClipRect.w - pcmd->ClipRect.y);
		//		commandBuffer.setScissor(0, scissor);

		//		// Bind font (and any UI) textures for this draw
		//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		//			*pipelineLayout, 0, *descriptorSet, {});

		//		// Issue indexed draw for this UI batch
		//		commandBuffer.drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
		//		indexOffset += pcmd->ElemCount;
		//	}

		//	vertexOffset += cmdList->VtxBuffer.Size;
		//}
		ImGui_ImplVulkan_RenderDrawData(data, *commandBuffer);
		commandBuffer.endRendering();
		pDevice->transition_image_layout(
			fontImage,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,              // dstStage
			vk::ImageAspectFlagBits::eColor
		);
		/*pDevice->transitionImageLayout(
			fontImage,
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			1
		);*/
		//commandBuffer.endRendering();
		commandBuffer.end();
	}
	void Gui::handleKey(int key, int scancode, int action, int mods) {
		ImGuiIO& io = ImGui::GetIO();

		// This example uses GLFW key codes and actions, but you can adapt this
		// to work with any windowing library's input system

		// Map the platform-specific key action to ImGui's key state
		// In GLFW: GLFW_PRESS = 1, GLFW_RELEASE = 0
		const int KEY_PRESSED = 1;  // Generic key pressed value
		const int KEY_RELEASED = 0; // Generic key released value

		if (action == KEY_PRESSED)
		{
			io.AddKeyEvent(static_cast<ImGuiKey>(key), true);
			std::cout << "key pressed \n";
		}
		if (action == KEY_RELEASED)
			io.AddKeyEvent(static_cast<ImGuiKey>(key), false);
		

		/*// Update modifier keys
		// These key codes are GLFW-specific, but you would use your windowing library's
		// equivalent key codes for other libraries
		const int KEY_LEFT_CTRL = 341;   // GLFW_KEY_LEFT_CONTROL
		const int KEY_RIGHT_CTRL = 345;  // GLFW_KEY_RIGHT_CONTROL
		const int KEY_LEFT_SHIFT = 340;  // GLFW_KEY_LEFT_SHIFT
		const int KEY_RIGHT_SHIFT = 344; // GLFW_KEY_RIGHT_SHIFT
		const int KEY_LEFT_ALT = 342;    // GLFW_KEY_LEFT_ALT
		const int KEY_RIGHT_ALT = 346;   // GLFW_KEY_RIGHT_ALT
		const int KEY_LEFT_SUPER = 343;  // GLFW_KEY_LEFT_SUPER
		const int KEY_RIGHT_SUPER = 347; // GLFW_KEY_RIGHT_SUPER

		io.KeyCtrl = io.KeysDown[KEY_LEFT_CTRL] || io.KeysDown[KEY_RIGHT_CTRL];
		io.KeyShift = io.KeysDown[KEY_LEFT_SHIFT] || io.KeysDown[KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[KEY_LEFT_ALT] || io.KeysDown[KEY_RIGHT_ALT];
		io.KeySuper = io.KeysDown[KEY_LEFT_SUPER] || io.KeysDown[KEY_RIGHT_SUPER];*/
	}

	bool Gui::getWantKeyCapture() {
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	void Gui::charPressed(uint32_t key) {
		ImGuiIO& io = ImGui::GetIO();
		io.AddInputCharacter(key);
	}
	void Gui::updateBuffers() {
		ImDrawData* drawData = ImGui::GetDrawData();
		if (!drawData || drawData->CmdListsCount == 0) {
			return;
		}

		// Calculate required buffer sizes
		vk::DeviceSize vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
		vk::DeviceSize indexBufferSize = drawData->TotalIdxCount * sizeof(ImDrawIdx);
		vk::raii::DeviceMemory memory({});
		vk::raii::DeviceMemory Indexmemory({});
		// Resize buffers if needed
		if (drawData->TotalVtxCount > vertexCount) {
			// Recreate vertex buffer with new size

			
			pDevice->createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertexBuffer, memory);
			vertexCount = drawData->TotalVtxCount;
		}

		if (drawData->TotalIdxCount > indexCount) {
			// Recreate index buffer with new size
			pDevice->createBuffer(indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indexBuffer, Indexmemory);
			indexCount = drawData->TotalIdxCount;
		}

		// Upload data to buffers
		ImDrawVert* vtxDst = static_cast<ImDrawVert*>(memory.mapMemory(0,vertexBufferSize));
		ImDrawIdx* idxDst = static_cast<ImDrawIdx*>(Indexmemory.mapMemory(0,indexBufferSize));

		for (int n = 0; n < drawData->CmdListsCount; n++) {
			const ImDrawList* cmdList = drawData->CmdLists[n];
			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}
		memory.unmapMemory();
		Indexmemory.unmapMemory();
	}
	void Gui::createDescriptorPool() {
				
		VkDescriptorPoolSize PoolSizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo PoolCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 1000 * IM_ARRAYSIZE(PoolSizes),
			.poolSizeCount = (uint32_t)IM_ARRAYSIZE(PoolSizes),
			.pPoolSizes = PoolSizes
		};

		VkResult res = vkCreateDescriptorPool(**device, &PoolCreateInfo, NULL, &descriptorPoolHandle);
		CheckVKResult(res);
			//VkResult res = vk::DescriptorPool(pDevice->getDevice(), &PoolCreateInfo, NULL, &descriptorPool);;
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

	void Gui::setStyle(uint32_t index) {
		ImGuiStyle& style = ImGui::GetStyle();

		switch (index) {
		case 0:
			// Custom Vulkan style
			style = vulkanStyle;
			break;
		case 1:
			// Classic style
			ImGui::StyleColorsClassic();
			break;
		case 2:
			// Dark style
			ImGui::StyleColorsDark();
			break;
		case 3:
			// Light style
			ImGui::StyleColorsLight();
			break;
		}
	}

	void Gui::initResources() {

		// Create the graphics pipeline with dynamic rendering
		// ... (shader loading, pipeline state setup, etc.)

		// For brevity, we're omitting the full pipeline creation code here
		// In a real implementation, you would:
		// 1. Load the vertex and fragment shaders
		// 2. Set up all the pipeline state (vertex input, input assembly, rasterization, etc.)
		// 3. Include the renderingInfo in the pipeline creation to enable dynamic rendering


		// Extract font atlas data from ImGui's internal font system
		// ImGui generates a texture atlas containing all glyphs needed for text rendering
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* fontData;                    // Raw pixel data from font atlas
		int texWidth, texHeight;                    // Dimensions of the generated font atlas
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		// Calculate total memory requirements for GPU transfer
		// Each pixel contains 4 bytes (RGBA) requiring precise memory allocation
		vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);
		// Define image dimensions and create extent structure
		
		// Create optimized GPU image for font texture storage
		// This image will be sampled by shaders during UI rendering
		
		pDevice->createImage(texWidth, texHeight, 1,vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal, fontImage, fontMemory);

		// Create image view for shader access
		vk::ImageViewCreateInfo imageViewCreateInfo{};
			imageViewCreateInfo.viewType = vk::ImageViewType::e2D,
			imageViewCreateInfo.format = vk::Format::eR8G8B8A8Unorm,
			imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor};
			imageViewCreateInfo.image = fontImage;
		// The image view defines how shaders interpret the raw image data
		fontImageView = vk::raii::ImageView(*device, imageViewCreateInfo);
		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory memory({});
		pDevice->createBuffer(uploadSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, memory);

		// Map staging buffer memory and copy font data
		// Direct memory mapping provides the fastest path for data transfer		
		
		/*createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);
		stagingBuffer.bindMemory(stagingBufferMemory, 0);*/
		void* dataStaging = memory.mapMemory(0, uploadSize);
		memcpy(dataStaging, fontData, uploadSize);
		memory.unmapMemory();
		// Transition image to optimal layout for data reception
		// Vulkan requires explicit layout transitions for optimal performance and correctness
		pDevice->transitionImageLayout(fontImage,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal , 1);
		//pDevice->transitionImageLayout(fontImage, vk::Format::eR8G8B8A8Unorm,
		//	vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0);
		// Execute the actual buffer-to-image copy operation
		// This transfers font data from staging buffer to the final GPU image
		pDevice->copyBufferToImage(stagingBuffer, fontImage,
			static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		// Transition image to shader-readable layout for rendering
		// Final layout optimization enables efficient sampling during UI rendering
		pDevice->transitionImageLayout(fontImage,
			vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal , 1);
		// Configure texture sampling parameters for optimal text rendering
   // These settings directly impact text quality and performance
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo.magFilter = vk::Filter::eLinear;                    // Smooth scaling when magnified
		samplerInfo.minFilter = vk::Filter::eLinear;                    // Smooth scaling when minified
		samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;        // Smooth transitions between mip levels
		samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;  // Prevent texture wrapping
		samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;  // Clean edge handling
		samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;  // 3D consistency
		samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;   // White border for clamped areas
		sampler = device->createSampler(samplerInfo);                   // Create the GPU sampler object

		// Create descriptor pool for shader resource binding
		// Descriptors provide the interface between shaders and GPU resources
		vk::DescriptorPoolSize poolSize{vk::DescriptorType::eCombinedImageSampler, 1};

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;     // Allow individual descriptor set freeing
		poolInfo.maxSets = 2;                                                      // Maximum number of descriptor sets
		poolInfo.poolSizeCount = 1;                                                // Number of pool size specifications
		poolInfo.pPoolSizes = &poolSize;                                           // Pool size configuration

		descriptorPool = device->createDescriptorPool(poolInfo);                   // Create descriptor pool

		// Create descriptor set layout defining shader resource interface
		// This layout must match the binding declarations in the ImGui shaders
		vk::DescriptorSetLayoutBinding binding{};
		binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;        // Combined texture and sampler
		binding.descriptorCount = 1;                                               // Single texture binding
		binding.stageFlags = vk::ShaderStageFlagBits::eFragment;                   // Used in fragment shader
		binding.binding = 0;                                                       // Shader binding point 0

		vk::DescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.bindingCount = 1;                                               // Number of bindings in layout
		layoutInfo.pBindings = &binding;                                           // Binding configuration array

		descriptorSetLayout = device->createDescriptorSetLayout(layoutInfo);       // Create layout object

		// Allocate descriptor set from pool using the defined layout
		// This creates the actual binding that connects GPU resources to shaders
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = *descriptorPool;                                // Source pool for allocation
		allocInfo.descriptorSetCount = 1;                                          // Number of sets to allocate
		vk::DescriptorSetLayout layouts[] = { *descriptorSetLayout };                // Layout template array
		allocInfo.pSetLayouts = layouts;                                           // Layout configuration

		descriptorSet = std::move(device->allocateDescriptorSets(allocInfo).front()); // Allocate and store set

		// Update descriptor set with actual font texture and sampler resources
		// This final step connects the physical GPU resources to the shader binding points
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;           // Expected image layout
		imageInfo.imageView = fontImageView;                           // Font texture view
		imageInfo.sampler = *sampler;                                              // Texture sampler

		vk::WriteDescriptorSet writeSet{};
		writeSet.dstSet = *descriptorSet;                                          // Target descriptor set
		writeSet.descriptorCount = 1;                                              // Number of resources to bind
		writeSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;       // Resource type
		writeSet.pImageInfo = &imageInfo;                                          // Image resource information
		writeSet.dstBinding = 0;                                                   // Binding point in shader

		device->updateDescriptorSets(writeSet, {});                   // Execute the binding update

		// Create pipeline cache
		vk::PipelineCacheCreateInfo pipelineCacheInfo{};
		pipelineCache = device->createPipelineCache(pipelineCacheInfo);

		// Create pipeline layout
		vk::PushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = vk::ShaderStageFlagBits::eVertex;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.setLayoutCount = 1;
		vk::DescriptorSetLayout setLayouts[] = { *descriptorSetLayout };
		pipelineLayoutInfo.pSetLayouts = setLayouts;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		pipelineLayout = device->createPipelineLayout(pipelineLayoutInfo);

		VkCommandBufferAllocateInfo cmdBufAllocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = NULL,
		.commandPool = *commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(commandBuffers.size())
		};
	}

	void Gui::createImages()
	{

	}
	void Gui::InitGUI() {
		IMGUI_CHECKVERSION();
		//IMGUI_DEBUG_LOG();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.DisplaySize.x = (float)fbWidth;
		io.DisplaySize.y = (float)fbHeight;
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		ImGui::GetStyle().FontScaleMain = 1.5f;
		setStyle(2);

		float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
		bool InstallGlfwCallbacks = true;
		ImGui_ImplGlfw_InitForVulkan(pDevice->getWindow(), InstallGlfwCallbacks);
		ImGuiStyle style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		colorFormat = pDevice->getSwapChainFormat();
		vk::PipelineRenderingCreateInfoKHR pipelineCreateInfo{ };
		pipelineCreateInfo.sType = vk::StructureType::ePipelineRenderingCreateInfoKHR;
		pipelineCreateInfo.pNext = NULL;
		pipelineCreateInfo.viewMask = 0;
		pipelineCreateInfo.colorAttachmentCount = 1;
		pipelineCreateInfo.pColorAttachmentFormats = &colorFormat;
		pipelineCreateInfo.depthAttachmentFormat = depthFormat;
		pipelineCreateInfo.stencilAttachmentFormat = vk::Format::eUndefined;
		
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
		init_info.Instance = **(pDevice->getInstance());
		init_info.PhysicalDevice = **(pDevice->getPhysicalDevice());
		init_info.Device = **(pDevice->getDevice());
		init_info.QueueFamily = (pDevice->getQueueFamily());
		init_info.DescriptorPool = descriptorPoolHandle;
		init_info.PipelineCache = NULL;
		init_info.PipelineInfoMain.PipelineRenderingCreateInfo =  pipelineCreateInfo;
		init_info.Queue = **(pDevice->getQueue());
		init_info.PipelineCache = *pipelineCache;
		init_info.MinImageCount = pDevice->minImageCount; //stuff
		init_info.UseDynamicRendering = true;
		init_info.ImageCount = pDevice->minImageCount;
		init_info.Allocator = NULL;
		init_info.PipelineInfoMain.RenderPass = NULL;
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = &CheckVKResult;// for debugging
		ImGui_ImplVulkan_Init(&init_info);
		vk::CommandPoolCreateInfo poolInfo{};
			poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			poolInfo.queueFamilyIndex = pDevice->getQueueIndex();
		commandPool = vk::raii::CommandPool(*device, poolInfo);
		vk::CommandBufferAllocateInfo allocInfo{};
			allocInfo.commandPool = commandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		commandBuffers = vk::raii::CommandBuffers(*device, allocInfo);
		
	}
}