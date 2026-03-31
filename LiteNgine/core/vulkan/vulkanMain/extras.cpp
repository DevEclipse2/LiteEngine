#include "../VulkanDevice.h"

namespace lte {

void VulkanDevice::ListFeatures(vk::PhysicalDeviceProperties* props,
	vk::PhysicalDeviceFeatures* features, vk::PhysicalDeviceMemoryProperties* memProps) {

	//lists all available features for funsies
	ConsoleLog::printU32("apiVersion", props->apiVersion);
	ConsoleLog::printU64("vendorID", props->vendorID);
	ConsoleLog::printU64("deviceID", props->deviceID);
	ConsoleLog::printStr("deviceName", props->deviceName);
	std::cout << "deviceType: " << static_cast<uint32_t>(props->deviceType) << "\n";
	ConsoleLog::printByteArrayHex("pipelineCacheUUID", props->pipelineCacheUUID, VK_UUID_SIZE);

	const auto& lim = props->limits;
	ConsoleLog::printU32("maxImageDimension1D", lim.maxImageDimension1D);
	ConsoleLog::printU32("maxImageDimension2D", lim.maxImageDimension2D);
	ConsoleLog::printU32("maxImageDimension3D", lim.maxImageDimension3D);
	ConsoleLog::printU32("maxImageArrayLayers", lim.maxImageArrayLayers);
	ConsoleLog::printU32("maxUniformBufferRange", lim.maxUniformBufferRange);
	ConsoleLog::printU32("maxStorageBufferRange", lim.maxStorageBufferRange);
	ConsoleLog::printU32("maxPushConstantsSize", lim.maxPushConstantsSize);
	ConsoleLog::printU32("maxMemoryAllocationCount", lim.maxMemoryAllocationCount);
	ConsoleLog::printU32("maxBoundDescriptorSets", lim.maxBoundDescriptorSets);

	ConsoleLog::printU32("maxComputeWorkGroupCount[0]", lim.maxComputeWorkGroupCount[0]);
	ConsoleLog::printU32("maxComputeWorkGroupCount[1]", lim.maxComputeWorkGroupCount[1]);
	ConsoleLog::printU32("maxComputeWorkGroupCount[2]", lim.maxComputeWorkGroupCount[2]);
	ConsoleLog::printU32("maxComputeWorkGroupInvocations", lim.maxComputeWorkGroupInvocations);

	ConsoleLog::printU32("maxSamplerAllocationCount", lim.maxSamplerAllocationCount);
	ConsoleLog::printU32("maxSamplerAnisotropy", lim.maxSamplerAnisotropy);
	ConsoleLog::printU32("bufferImageGranularity", lim.bufferImageGranularity);

	vk::PhysicalDeviceFeatures feats = *features;
	std::cout << "core features sample:\n";
	std::cout << "  geometryShader: " << feats.geometryShader << "\n";
	std::cout << "  tessellationShader: " << feats.tessellationShader << "\n";
	std::cout << "  samplerAnisotropy: " << feats.samplerAnisotropy << "\n";
	std::cout << "  shaderInt64: " << feats.shaderInt64 << "\n";
	std::cout << "  fillModeNonSolid: " << feats.fillModeNonSolid << "\n";
	std::cout << "  multiDrawIndirect: " << feats.multiDrawIndirect << "\n";

	vk::PhysicalDeviceMemoryProperties memoryProperties = *memProps;
	ConsoleLog::printU32("memoryHeap count", memProps->memoryHeapCount);
	for (uint32_t i = 0; i < memProps->memoryHeapCount; ++i) {
		std::cout << "  heap[" << i << "].size: " << memProps->memoryHeaps[i].size << "\n";
		std::cout << "  heap[" << i << "].flags: " << static_cast<uint32_t>(memProps->memoryHeaps[i].flags) << "\n";
	}

	std::cout << "memoryType count: " << memProps->memoryTypeCount << "\n";
	for (uint32_t i = 0; i < memProps->memoryTypeCount; ++i) {
		std::cout << "  type[" << i << "].propertyFlags: " << static_cast<uint32_t>(memProps->memoryTypes[i].propertyFlags)
			<< " heapIndex: " << memProps->memoryTypes[i].heapIndex << "\n";
	}
}
}