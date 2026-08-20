#include "DebugMessenger.h"
#include "../forScrap/Lt_Console.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
	vk::DebugUtilsMessageTypeFlagsEXT              type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{


	uint8_t logSeverity = 0;
	switch (severity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		//verbose
		lte::Con::Log("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage, TAG_VULKAN);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		//info
		lte::Con::Log("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage, TAG_VULKAN);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		//warning
		lte::Con::LogWarning("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage, TAG_VULKAN);
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		lte::Con::LogError("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage,UDEF_SEVERITY, TAG_VULKAN);

		break;
	}
	return vk::False;
}

namespace ltCore {
	//validation layer stuff

	void DebugMessenger::setupMessenger(vk::raii::Instance& instance)
	{

		//sets it up for both warnings AND errors using container

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

		//this makes the message
		//idfk why debug callback aint wokring
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
			debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags,
			debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags,
			debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &::debugCallback;
		debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}
}

