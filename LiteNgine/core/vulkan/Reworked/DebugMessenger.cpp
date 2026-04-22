#include "DebugMessenger.h"
namespace lte {

	DebugMessenger::DebugMessenger() {

	}
	DebugMessenger::~DebugMessenger() {

	}
	//validation layer stuff
	VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugMessenger::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT       severity,
		vk::DebugUtilsMessageTypeFlagsEXT              type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

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