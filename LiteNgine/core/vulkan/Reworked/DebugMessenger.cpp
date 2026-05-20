#include "DebugMessenger.h"
#include "../EngineClasses/Lt_Console.h"
namespace lte {

	vk::DebugUtilsMessengerEXT DebugMessenger::debugMessenger;


	DebugMessenger::DebugMessenger() {

	}
	DebugMessenger::~DebugMessenger() {

	}
	//validation layer stuff
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
		Con::Log("validation layer: type " + to_string(type) + " msg: " + pCallbackData->pMessage, logSeverity);
		return vk::False;
	}

	void DebugMessenger::setupMessenger(vk::raii::Instance* instance)
	{

		//sets it up for both warnings AND errors using container
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		//warn types
		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		//this makes the message
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
		debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags,
			debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags,
			debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;
		debugMessenger = instance->createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}


	


}