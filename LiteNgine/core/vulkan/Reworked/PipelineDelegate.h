#pragma once
#include "../VulkanDevice.h"
#include <vulkan/vulkan_raii.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
namespace lte {
	struct LtPipeline {
		//something that contains all of the necessary information about a particular pipeline
		vk::raii::Pipeline				pipeline = nullptr;
		vk::DescriptorSetLayout			descSetLayout;
		const void createPipeline(uint32_t stageCount, vk::PipelineShaderStageCreateInfo* shaderStages, vk::PipelineVertexInputStateCreateInfo* vertexInputInfo, vk::PipelineInputAssemblyStateCreateInfo* inputAssembly,
			vk::PipelineViewportStateCreateInfo* viewportState, vk::PipelineRasterizationStateCreateInfo* rasterizer, vk::PipelineMultisampleStateCreateInfo* multisampling, vk::PipelineColorBlendStateCreateInfo* colorBlending,
			vk::PipelineDynamicStateCreateInfo* dynamicState, vk::PipelineDepthStencilStateCreateInfo* depthStencil, vk::raii::PipelineLayout* pipelineLayout, uint32_t colorAttachmentCount, vk::Format* colorAttachmentFormats, vk::Format depthAttachmentFormat, vk::raii::Device* device) {
			vk::GraphicsPipelineCreateInfo graphicsInfo{};
			graphicsInfo.stageCount = stageCount,
				graphicsInfo.pStages = shaderStages,
				graphicsInfo.pVertexInputState = vertexInputInfo,
				graphicsInfo.pInputAssemblyState = inputAssembly,
				graphicsInfo.pViewportState = viewportState,
				graphicsInfo.pRasterizationState = rasterizer,
				graphicsInfo.pMultisampleState = multisampling,
				graphicsInfo.pColorBlendState = colorBlending,
				graphicsInfo.pDynamicState = dynamicState,
				graphicsInfo.pDepthStencilState = depthStencil,
				graphicsInfo.layout = *pipelineLayout,
				graphicsInfo.renderPass = nullptr;

			vk::PipelineRenderingCreateInfo renderingInfo{};
			renderingInfo.colorAttachmentCount = colorAttachmentCount,
				renderingInfo.pColorAttachmentFormats = colorAttachmentFormats,
				renderingInfo.depthAttachmentFormat = depthAttachmentFormat;

			vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain{ graphicsInfo , renderingInfo };
			pipeline = vk::raii::Pipeline(*device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
		}
	};
	static class PipelineDelegate
	{
		
		public:
			static void createPipelineFast(LtPipeline* pipeline, std::string shaderFilepath, std::string vertShadername, std::string fragShadername, vk::raii::Device* device, vk::raii::PhysicalDevice* physicalDevice, vk::SurfaceFormatKHR* surfaceformat, vk::DescriptorSetLayout layout);
			static vk::raii::ShaderModule createShaderModule(const std::vector<char>& code , vk::raii::Device* pDevice);
			static std::vector<char> readShaderInfo(std::vector<char>* output, std::string filepath);
			static void createShaderStage(vk::PipelineShaderStageCreateInfo* info , vk::ShaderStageFlagBits flags , vk::ShaderModule* pModule , std::string* pName);
			static void createVertexInputInfo(vk::PipelineVertexInputStateCreateInfo* pInfo, vk::PipelineShaderStageCreateInfo* Vertex, vk::PipelineShaderStageCreateInfo* Fragment, void* getBindingDescFunc, void* getAttributeDescFunce);
			static vk::Format findDepthFormat(vk::raii::PhysicalDevice* device);
			static vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features, vk::raii::PhysicalDevice* physicalDevice);
			static void createDescriptorSetLayout(vk::DescriptorSetLayout* descriptorSetLayout, vk::raii::Device* device);
		//private:
	};
}


