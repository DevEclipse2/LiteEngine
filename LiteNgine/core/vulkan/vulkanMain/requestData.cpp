#include "../VulkanDevice.h"
namespace lte {
	vk::raii::Pipeline* VulkanDevice::getPipeline() {return &graphicsPipeline;}
	vk::raii::Instance* VulkanDevice::getInstance() { return &instance;}
	vk::raii::Queue* VulkanDevice::getQueue() { return &queue;}
	uint32_t  VulkanDevice::getQueueFamily() { return queueIndex;}
	vk::raii::Device* VulkanDevice::getDevice() {return &device;}
	GLFWwindow* VulkanDevice::getWindow() {return window.getGLFWWindow();}
	void VulkanDevice::getFrameBufferSize(int* width, int* height)
	{
		glfwGetFramebufferSize(window.getGLFWWindow(), width, height);
		WIDTH = *width;
		HEIGHT = *height;};

	vk::Format VulkanDevice::getSwapChainFormat() {return swapChainSurfaceFormat.format;}
	vk::raii::PhysicalDevice* VulkanDevice::getPhysicalDevice() {return &physicalDevice;}
	uint32_t* VulkanDevice::getpFrameIndex() { return &frameIndex; }
	std::vector<vk::raii::Image>* VulkanDevice::getpImages() { return &imagesArr; }
	vk::Format					VulkanDevice::getDepthFormat() { return findDepthFormat(); }
	vk::raii::Image* VulkanDevice::getpColorImage() { return &colorImage; }
	vk::raii::DeviceMemory* VulkanDevice::getpColorImageMemory() { return &colorImageMemory; }
	vk::raii::ImageView* VulkanDevice::getpColorImageView() { return &colorImageView; }
	vk::raii::ImageView* VulkanDevice::getDepthImageView() { return &depthImageView; }
	std::vector<vk::raii::ImageView>* VulkanDevice::getSwapChainImageViews() { return &swapChainImageViews; }

	void VulkanDevice::setGuiCommandBuffers(std::vector<vk::raii::CommandBuffer>* pCmd) {
		pUiCommandBuffer = pCmd;
	}
	void VulkanDevice::setGuiRef(Gui* pGui) {
		gui = pGui;
	}
	uint32_t				VulkanDevice::getQueueIndex() { return queueIndex; }
	void VulkanDevice::getProfilingData(uint32_t* pFPS, float* pFrametime, uint64_t* pvertices, uint64_t* pindices, uint64_t* pmodels) { 
		*pFPS		= fps;
		*pFrametime = frameTime;
		uint64_t verticeList = 0; 
		for (const auto& vList : vertices) {
			verticeList += vList.size();
		}
		
		*pvertices	= verticeList; 
		verticeList = 0;
		for (const auto& vList : indices) {
			verticeList += vList.size();
		}
		*pindices	= verticeList;
		*pmodels	= models.size();
	}
	vk::Image* VulkanDevice::getImage(uint32_t index) {return &swapChainImages[index];}
	vk::raii::ImageView* VulkanDevice::getImageView(uint32_t index) { return &swapChainImageViews[index]; }

}