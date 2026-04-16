#include "Gui.h"
namespace lte{
	Gui::Gui(VulkanDevice* vulkDev) : pDevice{ vulkDev }
	{
		pDevice->getFrameBufferSize(&fbWidth, &fbHeight);
		createDescriptorPool();
		InitGUI();
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
		if (device) {
			device->waitIdle();
		}

		// All resources are automatically cleaned up by their destructors
		// No manual cleanup needed

		// ImGui context is destroyed separately
	}
	void Gui::Init(VulkanDevice* device)
	{
		
	}
	void Gui::createDescriptorPool() {
				
		vk::DescriptorPoolSize poolSizes[] = {
			{vk::DescriptorType::eSampler, 1000},
			{vk::DescriptorType::eCombinedImageSampler , 1000},
			{vk::DescriptorType::eSampledImage ,1000},
			{vk::DescriptorType::eStorageImage ,1000},
			{vk::DescriptorType::eUniformTexelBuffer ,1000},
			{vk::DescriptorType::eStorageTexelBuffer ,1000},
			{vk::DescriptorType::eUniformBuffer ,1000},
			{vk::DescriptorType::eStorageBuffer ,1000},
			{vk::DescriptorType::eUniformBufferDynamic ,1000},
			{vk::DescriptorType::eStorageBufferDynamic ,1000},
			{vk::DescriptorType::eInputAttachment , 1000} };
		
		vk::DescriptorPoolCreateInfo PoolCreateInfo = {};
			PoolCreateInfo.sType = vk::StructureType::eDescriptorPoolCreateInfo,
			PoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
			PoolCreateInfo.maxSets = 1000 * IM_ARRAYSIZE(poolSizes),
			PoolCreateInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes),
			PoolCreateInfo.pPoolSizes = poolSizes;

			descriptorPool = vk::raii::DescriptorPool(*(pDevice->getDevice()), PoolCreateInfo);
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
		
		pDevice->createImage(texWidth, texHeight, 0,vk::SampleCountFlagBits::e1, vk::Format::eR8G8B8A8Unorm,
			vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal, fontImage, fontMemory);

		// Create image view for shader access

		vk::ImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.viewType = vk::ImageViewType::e2D,
			imageViewCreateInfo.format = vk::Format::eR8G8B8A8Unorm,
			imageViewCreateInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor};
		imageViewCreateInfo.image = fontImage;
		// The image view defines how shaders interpret the raw image data
		fontImageView = vk::raii::ImageView(*(pDevice->getDevice()), imageViewCreateInfo);
		vk::raii::Buffer stagingBuffer = nullptr;
		vk::raii::DeviceMemory memory = nullptr;
		pDevice->createBuffer(uploadSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, memory);

			

		// Map staging buffer memory and copy font data
		// Direct memory mapping provides the fastest path for data transfer
		void* data = **stagingBuffer.map();                          // Map GPU memory to CPU address space
		memcpy(data, fontData, uploadSize);                        // Copy font atlas data to GPU memory
		stagingBuffer.unmap();
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
		ImGui::StyleColorsDark();

		float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
		ImGuiStyle style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.
		setStyle(2);
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}


		bool InstallGlfwCallbacks = true;
		ImGui_ImplGlfw_InitForVulkan(pDevice->getWindow(), InstallGlfwCallbacks);
		vk::Format colorFormat = pDevice->getSwapChainFormat();
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_3;              // Pass in your value of VkApplicationInfo::apiVersion, otherwise will default to header version.
		init_info.Instance = **(pDevice->getInstance());
		init_info.PhysicalDevice = **(pDevice->getPhysicalDevice());
		init_info.Device = **(pDevice->getDevice());
		init_info.QueueFamily = (pDevice->getQueueFamily());
		init_info.Queue = **(pDevice->getQueue());
		init_info.PipelineCache = *pipelineCache;
		init_info.DescriptorPool = *descriptorPool;
		init_info.MinImageCount = pDevice->minImageCount; //stuff
		init_info.UseDynamicRendering = true;
		init_info.ImageCount = pDevice->minImageCount;
		//init_info.Allocator = g_Allocator;
		//init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
		//init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info);

	}
}