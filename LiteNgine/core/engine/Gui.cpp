#include "Gui.h"
namespace lte{
	Gui::Gui(vk::raii::Device& device, vk::raii::PhysicalDevice& physicalDevice,
		vk::raii::Queue& graphicsQueue, uint32_t graphicsQueueFamily)
		: device(&device), physicalDevice(&physicalDevice),
		graphicsQueue(&graphicsQueue), graphicsQueueFamily(graphicsQueueFamily),
		// Initialize buffers directly
		vertexBuffer(*device, 1,
			,
		indexBuffer(*device, 1,
			vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent) 
	{
		vk::BufferCreateInfo info({}, 1 ,vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		vertexBuffer = device.createBuffer(info);
		// Set up dynamic rendering info
		renderingInfo.colorAttachmentCount = 1;
		vk::Format formats[] = { colorFormat };
		renderingInfo.pColorAttachmentFormats = &colorFormat;
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
		pDevice = device;
		pDevice->getFrameBufferSize(&fbWidth, &fbHeight);
		createDescriptorPool();
		InitGUI();
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


	void Gui::InitGUI() {
		ImGui::CreateContext();
		ImGuiIO io = ImGui::GetIO(); // important stuff
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
		io.DisplaySize.x = (float)fbWidth;
		io.DisplaySize.y = (float)fbHeight;

		ImGui::GetStyle().FontScaleMain = 1.5f;
		ImGui::StyleColorsDark();

		float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
		style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
		io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
		io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear ImGui and Platform Windows when Monitor DPI changes.

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
		init_info.MinImageCount = 2; //stuff
		init_info.ImageCount = wd->ImageCount;
		init_info.Allocator = g_Allocator;
		init_info.PipelineInfoMain.RenderPass = wd->RenderPass;
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info);

	}
}