#include "Lt_Gui.h"

void CheckVKResult(VkResult err) {
	if (err == 0) {
		return;
	}
	std::cout << stderr << "[vulkan] error : VkResult = %d\n" << err;
	if (err < 0) {
		abort();
	}
}

namespace lte {
	void Lt_Gui::Instantiate()
	{
		createDescriptorPool();
		createImages();
	}

	void Lt_Gui::setStyle(char index)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		switch (index) {
		case 0:
			// Custom Vulkan style
			//style = vulkanStyle;

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
	void Lt_Gui::InitGui(Lt_GuiCreationInfo& info) 
	{
		creationInfo = std::move(info);
		IMGUI_CHECKVERSION();
		//IMGUI_DEBUG_LOG();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.DisplaySize.x = (float)creationInfo.width;
		io.DisplaySize.y = (float)creationInfo.height;
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		ImGui::GetStyle().FontScaleMain = 1.5f;
		setStyle(2);

		float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
		bool InstallGlfwCallbacks = true;
		ImGui_ImplGlfw_InitForVulkan(creationInfo.window, InstallGlfwCallbacks);
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

		colorFormat = creationInfo.colorFormat;
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
		init_info.Instance = **(creationInfo.instance);
		init_info.PhysicalDevice = **(creationInfo.physicalDevice);
		init_info.Device = **(creationInfo.device);
		init_info.QueueFamily = (creationInfo.queueFamily);
		init_info.DescriptorPool = descriptorPoolHandle;
		//init_info.PipelineCache = NULL;
		init_info.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineCreateInfo;
		init_info.Queue = **(creationInfo.queue);
		init_info.PipelineCache = **creationInfo.cache;
		init_info.MinImageCount = creationInfo.minImgCount; //stuff
		init_info.UseDynamicRendering = true;
		init_info.ImageCount = creationInfo.minImgCount;
		init_info.Allocator = NULL;
		init_info.PipelineInfoMain.RenderPass = NULL;
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_8_BIT;
		init_info.CheckVkResultFn = &CheckVKResult;// for debugging
		ImGui_ImplVulkan_Init(&init_info);



		//command pools and command buffers are made in lt_Vulkan

		/*vk::CommandPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		poolInfo.queueFamilyIndex = pDevice->getQueueIndex();
		commandPool = vk::raii::CommandPool(*device, poolInfo);
		vk::CommandBufferAllocateInfo allocInfo{};
		allocInfo.commandPool = commandPool,
			allocInfo.level = vk::CommandBufferLevel::ePrimary,
			allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
		commandBuffers = vk::raii::CommandBuffers(*device, allocInfo);*/
	}
	void Lt_Gui::createImages() 
	{

	}
	void Lt_Gui::createResources()
	{
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

		LtImage fontImage{};
		ImageDelegate::createImage(fontImage, texWidth, texHeight, 1, vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal,*creationInfo.device, *creationInfo.physicalDevice);
		ImageDelegate::createImageView(fontImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor, 1, *creationInfo.device);
		fontImgIndex = ImageDelegate::requestImageCreation(fontImage);
		vk::raii::Buffer stagingBuffer({});
		vk::raii::DeviceMemory memory({});
		Buffers::createBuffer(uploadSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, memory, *creationInfo.device, *creationInfo.physicalDevice);

		// Map staging buffer memory and copy font data
		// Direct memory mapping provides the fastest path for data transfer		

		void* dataStaging = memory.mapMemory(0, uploadSize);
		memcpy(dataStaging, fontData, uploadSize);
		memory.unmapMemory();
		singleTimeCommandInfo singleTCommandInfo{ creationInfo.device , creationInfo.commandPool, creationInfo.queue};
		// Transition image to optimal layout for data reception
		// Vulkan requires explicit layout transitions for optimal performance and correctness
		ImageDelegate::transitionImageLayout(ImageDelegate::ImagePool[fontImgIndex]->image,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 1,singleTCommandInfo);
		//pDevice->transitionImageLayout(fontImage, vk::Format::eR8G8B8A8Unorm,
		//	vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 0);
		// Execute the actual buffer-to-image copy operation
		// This transfers font data from staging buffer to the final GPU image
		Buffers::copyBufferToImage(stagingBuffer, ImageDelegate::ImagePool[fontImgIndex]->image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), singleTCommandInfo);
		// Transition image to shader-readable layout for rendering
		// Final layout optimization enables efficient sampling during UI rendering
		ImageDelegate::transitionImageLayout(ImageDelegate::ImagePool[fontImgIndex]->image,
			vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, 1, singleTCommandInfo);
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
		sampler = vk::raii::Sampler(*creationInfo.device,samplerInfo);                   // Create the GPU sampler object

		// Create descriptor pool for shader resource binding
		// Descriptors provide the interface between shaders and GPU resources
		vk::DescriptorPoolSize poolSize{ vk::DescriptorType::eCombinedImageSampler, 1 };

		vk::DescriptorPoolCreateInfo poolInfo{};
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;     // Allow individual descriptor set freeing
		poolInfo.maxSets = 2;                                                      // Maximum number of descriptor sets
		poolInfo.poolSizeCount = 1;                                                // Number of pool size specifications
		poolInfo.pPoolSizes = &poolSize;                                           // Pool size configuration

		descriptorPool = vk::raii::DescriptorPool(*creationInfo.device, poolInfo);                   // Create descriptor pool

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

		descriptorSetLayout = vk::raii::DescriptorSetLayout(*creationInfo.device,layoutInfo);       // Create layout object

		// Allocate descriptor set from pool using the defined layout
		// This creates the actual binding that connects GPU resources to shaders
		vk::DescriptorSetAllocateInfo allocInfo{};
		allocInfo.descriptorPool = *descriptorPool;                                // Source pool for allocation
		allocInfo.descriptorSetCount = 1;                                          // Number of sets to allocate
		vk::DescriptorSetLayout layouts[] = { *descriptorSetLayout };                // Layout template array
		allocInfo.pSetLayouts = layouts;                                           // Layout configuration

		descriptorSet = std::move(creationInfo.device->allocateDescriptorSets(allocInfo).front()); // Allocate and store set

		// Update descriptor set with actual font texture and sampler resources
		// This final step connects the physical GPU resources to the shader binding points
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;			// Expected image layout
		imageInfo.imageView = ImageDelegate::ImagePool[fontImgIndex]->imageView;	// Font texture view
		imageInfo.sampler = *sampler;												// Texture sampler

		vk::WriteDescriptorSet writeSet{};
		writeSet.dstSet = *descriptorSet;                                          // Target descriptor set
		writeSet.descriptorCount = 1;                                              // Number of resources to bind
		writeSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;       // Resource type
		writeSet.pImageInfo = &imageInfo;                                          // Image resource information
		writeSet.dstBinding = 0;                                                   // Binding point in shader

		creationInfo.device->updateDescriptorSets(writeSet, {});                   // Execute the binding update

		// Create pipeline cache
		vk::PipelineCacheCreateInfo pipelineCacheInfo{};
		creationInfo.cache = &creationInfo.device->createPipelineCache(pipelineCacheInfo);

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

	void Lt_Gui::createDescriptorPool()
	{
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
	}
}