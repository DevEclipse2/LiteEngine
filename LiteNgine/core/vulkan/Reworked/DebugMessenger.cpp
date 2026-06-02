#include "DebugMessenger.h"
#include "../EngineClasses/Lt_Console.h"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
	vk::DebugUtilsMessageTypeFlagsEXT              type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{

	std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

	uint8_t logSeverity = 0;
	switch (severity) {
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
		//verbose
		logSeverity = (uint8_t)LOG_VERBOSE;
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
		//verbose
		logSeverity = (uint8_t)LOG_INFO;
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
		//verbose
		logSeverity = (uint8_t)LOG_WARN;
		break;
	case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
		//verbose
		logSeverity = (uint8_t)LOG_ERR;
		break;
	}
	lte::Con::Log("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage, logSeverity);
	return vk::False;
}

namespace lte {

	vk::raii::DebugUtilsMessengerEXT DebugMessenger::debugMessenger = nullptr;



	DebugMessenger::DebugMessenger() {

	}
	DebugMessenger::~DebugMessenger() {

	}
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

