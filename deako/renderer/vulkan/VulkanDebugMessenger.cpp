#include "deako_pch.h"
#include "VulkanDebugMessenger.h"

#include "VulkanInstance.h"

namespace Deako {

	static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		// Convert message severity to string
		std::string severityString;
		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:   severityString = "VERBOSE"; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:      severityString = "INFO"; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:   severityString = "WARNING"; break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:     severityString = "ERROR"; break;
			default:                                                severityString = "UNKNOWN SEVERITY";
		}

		// Convert message type to string
		std::string typeString;
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)      typeString += "GENERAL";
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)   typeString += "VALIDATION";
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)  typeString += "PERFORMANCE";

		std::string message = "[[VULKAN " + typeString + " " + severityString + "]]:\n " + pCallbackData->pMessage + "\n";

		switch (messageSeverity)
		{
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				DK_CORE_TRACE(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				DK_CORE_INFO(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				DK_CORE_WARN(message.c_str());
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				DK_CORE_ERROR(message.c_str());
				break;
			default:
				DK_CORE_ERROR("Unknown severity level: {}", message.c_str());
				break;
		}

		return VK_FALSE;
	}

	VulkanDebugMessenger::VulkanDebugMessenger(VkInstance instance)
		: m_Messenger(VK_NULL_HANDLE), m_Instance(instance)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = VulkanDebugCallback;

		auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");

		if (vkCreateDebugUtilsMessengerEXT)
		{
			VK_CHECK(vkCreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_Messenger));
		}
	}

	VulkanDebugMessenger::~VulkanDebugMessenger()
	{
		auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");

		if (vkDestroyDebugUtilsMessengerEXT)
		{
			vkDestroyDebugUtilsMessengerEXT(m_Instance, m_Messenger, nullptr);
		}

		DK_CORE_INFO("VulkanDebugMessenger destroyed!");
	}

	std::string VulkanErrorString(VkResult errorCode)
	{
		switch (errorCode)
		{
#define STR(r) case VK_ ##r: return #r
			STR(NOT_READY);
			STR(TIMEOUT);
			STR(EVENT_SET);
			STR(EVENT_RESET);
			STR(INCOMPLETE);
			STR(ERROR_OUT_OF_HOST_MEMORY);
			STR(ERROR_OUT_OF_DEVICE_MEMORY);
			STR(ERROR_INITIALIZATION_FAILED);
			STR(ERROR_DEVICE_LOST);
			STR(ERROR_MEMORY_MAP_FAILED);
			STR(ERROR_LAYER_NOT_PRESENT);
			STR(ERROR_EXTENSION_NOT_PRESENT);
			STR(ERROR_FEATURE_NOT_PRESENT);
			STR(ERROR_INCOMPATIBLE_DRIVER);
			STR(ERROR_TOO_MANY_OBJECTS);
			STR(ERROR_FORMAT_NOT_SUPPORTED);
			STR(ERROR_SURFACE_LOST_KHR);
			STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
			STR(SUBOPTIMAL_KHR);
			STR(ERROR_OUT_OF_DATE_KHR);
			STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
			STR(ERROR_VALIDATION_FAILED_EXT);
			STR(ERROR_INVALID_SHADER_NV);
			STR(ERROR_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef STR
			default:
				return "UNKNOWN_ERROR";
		}

	}

}
