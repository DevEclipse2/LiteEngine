#pragma once
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#define IMGUI_IMPL_VULKAN_HAS_DYNAMIC_RENDERING
#include "../../../dep/backends/imgui.h"
#include "../../../dep/backends/imgui_impl_glfw.h"
#include "../../../dep/backends/imgui_impl_vulkan.h"
#include <GLFW/glfw3.h>
#include "../Reworked/ImageDelegate.h"
#include "../Reworked/Buffers.h"
//#include "../Reworked/LtSync.h"
#include <vector>
#include <iostream>
//this should be a static thing and should only have 1! instance ever
//or at most 2 because i cannot tell you what to do.
// a slightly better (or worse) implementation of GUI
namespace lte 
{
	struct PushConstBlock {
		glm::vec2 scale;                                    // UI scaling factors for different screen sizes
		glm::vec2 translate;                                // Translation offset for UI positioning
	} pushConstBlock;
	struct Lt_GuiCreationInfo 
	{
		int width = 0;
		int height = 0;
		vk::raii::Device* device;
		vk::raii::PhysicalDevice* physicalDevice;
		GLFWwindow* window;
		vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;
		vk::raii::PipelineCache* cache = nullptr;
		uint32_t queueFamily = 0;
		vk::raii::Queue* queue = nullptr;
		uint32_t minImgCount = 0;
		vk::raii::Instance* instance = nullptr;
		vk::raii::CommandPool* commandPool = nullptr;
		uint8_t maxFramesInFlight = 3;
		std::vector<vk::raii::ImageView>* pImageViews = nullptr;
		//physical device
		//command pool
		//command buffers
		//window data
		uint32_t* colorImageViewIndex = 0;
		vk::raii::Pipeline* pipeline;


	};
	class Lt_Gui
	{
	public:

		void Instantiate();
		bool drawFrame(char frameIndex);
		void updateBuffers();
		void updateFrameBuffer(int width, int height);
		bool firstFrame = true;
		void handleKey(int key, int scancode, int action, int mods);
		void charPressed(uint32_t key);
		bool getWantKeyCapture();
		void InitGui(Lt_GuiCreationInfo& info);
		std::vector<vk::raii::CommandBuffer> commandBuffers = {};
		void Terminate();
	private:
		VkDescriptorPool descriptorPoolHandle;
		void createDescriptorPool();
		void createImages();
		void setStyle(char index);
		vk::Format colorFormat = vk::Format::eB8G8R8A8Unorm;   // Target framebuffer format
		vk::Format depthFormat;
		void recordCommandBuffer(ImDrawData* data, uint8_t index);
		void doDynamicRendering(vk::raii::CommandBuffer& commandBuffer, ImDrawData* data, char drawindex);

		void createResources();
		uint32_t fontImgIndex;
		Lt_GuiCreationInfo creationInfo;
		vk::raii::Sampler sampler = nullptr;
		vk::raii::DescriptorPool descriptorPool = nullptr;
		vk::raii::DescriptorSetLayout descriptorSetLayout = nullptr;
		vk::raii::DescriptorSet descriptorSet = nullptr;
		vk::raii::PipelineLayout pipelineLayout = nullptr;

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		vk::raii::Buffer vertexBuffer;
		vk::raii::Buffer indexBuffer;
		int fbWidth = 0;
		int fbHeight = 0;
	};

}