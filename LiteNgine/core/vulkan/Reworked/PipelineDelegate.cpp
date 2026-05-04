#include "PipelineDelegate.h"
namespace lte {

	void PipelineDelegate::createPipelineFast (LtPipeline* pipeline , std::string shaderFilepath, std::string vertShadername , std::string fragShadername , vk::raii::Device& device , vk::raii::PhysicalDevice& physicalDevice, vk::SurfaceFormatKHR* surfaceformat,vk::raii::DescriptorSetLayout& layout) 
	{
		//this creates the very basic pipeline for general use
		vk::PipelineShaderStageCreateInfo vertShaderInfo{};
		vk::PipelineShaderStageCreateInfo fragShaderInfo{};
		vk::ShaderModule module = createShaderModule(readShaderInfo(nullptr, shaderFilepath), device);
		createShaderStage(vertShaderInfo, vk::ShaderStageFlagBits::eVertex		, &module, &vertShadername);
		createShaderStage(fragShaderInfo, vk::ShaderStageFlagBits::eFragment	, &module, &fragShadername);
		vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderInfo, fragShaderInfo };


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
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1,
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
			pipelineLayoutInfo.pSetLayouts = &*layout,
			pipelineLayoutInfo.pushConstantRangeCount = 0;

		vk::raii::PipelineLayout pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutInfo);

		vk::Format depthFormat = findDepthFormat(physicalDevice);
		pipeline->createPipeline(std::size(shaderStages),shaderStages,&vertexInputInfo,&inputAssembly,&viewportState,&rasterizer,&multisampling,&colorBlending,&dynamicState,&depthStencil,pipelineLayout,1,&(surfaceformat->format), depthFormat,device);

	}
	std::vector<char> PipelineDelegate::readShaderInfo(std::vector<char>* output, std::string filepath)
	{
		std::ifstream file(filepath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}
		std::vector<char> buffer(file.tellg());
		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
		file.close();
		if (output != nullptr) {
			*output = buffer;
		}
		return buffer;
		
	}
	void PipelineDelegate::createShaderStage(vk::PipelineShaderStageCreateInfo& info, vk::ShaderStageFlagBits flags, vk::ShaderModule* pModule, std::string* pName)
	{
		info.stage = flags;
		info.module = *pModule;
		info.pName = pName->c_str();
	}
	void PipelineDelegate::createVertexInputInfo(vk::PipelineVertexInputStateCreateInfo* pInfo, vk::PipelineShaderStageCreateInfo* Vertex, vk::PipelineShaderStageCreateInfo* Fragment, void* getBindingDescFunc, void* getAttributeDescFunce)
	{

	}
	[[nodiscard]] vk::raii::ShaderModule PipelineDelegate::createShaderModule(const std::vector<char>& code , vk::raii::Device& pDevice)
	{

		vk::ShaderModuleCreateInfo createInfo{};
		createInfo.codeSize = code.size() * sizeof(char),
			createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		//createInfo.structureType = VK_KHR_shader_draw_parameters;
		vk::raii::ShaderModule shaderModule{pDevice,createInfo }; //validationlayerError
		return shaderModule;
	}
	vk::Format PipelineDelegate::findDepthFormat(vk::raii::PhysicalDevice& device) {
		return findSupportedFormat(
			{ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
			vk::ImageTiling::eOptimal,
			vk::FormatFeatureFlagBits::eDepthStencilAttachment, device
		);
	}
	vk::Format PipelineDelegate::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features , vk::raii::PhysicalDevice& physicalDevice) {
		auto formatIt = std::ranges::find_if(candidates, [&](auto const format) {
			vk::FormatProperties props = physicalDevice.getFormatProperties(format);
			return (((tiling == vk::ImageTiling::eLinear) && ((props.linearTilingFeatures & features) == features)) ||
				((tiling == vk::ImageTiling::eOptimal) && ((props.optimalTilingFeatures & features) == features)));
			});
		if (formatIt == candidates.end())
		{
			throw std::runtime_error("failed to find supported format!");
		}
		return *formatIt;
	}
	void PipelineDelegate::createDescriptorSetLayout(vk::raii::DescriptorSetLayout& descriptorSetLayout, vk::raii::Device& device) 
	{

		std::array bindings = {
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr)
		};
		vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.size(), bindings.data());
		descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
	}

}