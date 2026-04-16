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

}