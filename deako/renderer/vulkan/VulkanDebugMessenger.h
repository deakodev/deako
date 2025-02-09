#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class VulkanInstance;

	class VulkanDebugMessenger : public Inheritor<Object, VulkanDebugMessenger>
	{
	public:
		VulkanDebugMessenger(VkInstance instance);
		~VulkanDebugMessenger();

		operator VkDebugUtilsMessengerEXT() const { return m_Messenger; }

	private:
		VkDebugUtilsMessengerEXT m_Messenger;

		VkInstance m_Instance;
	};

	DK_TYPE_NAME(VulkanDebugMessenger);

	std::string VulkanErrorString(VkResult errorCode);

}

#define VK_CHECK(f)																				        \
{																										\
    VkResult res = (f);																					\
    if (res != VK_SUCCESS)																				\
    {																									\
        std::string errorMessage = "Fatal : VkResult is " + VulkanErrorString(res) +                    \
			" in " + __FILE__ + " at line " + std::to_string(__LINE__);                                 \
        DK_CORE_ASSERT(res == VK_SUCCESS, errorMessage);												\
    }																									\
}


