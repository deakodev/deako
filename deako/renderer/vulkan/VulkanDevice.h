#pragma once

#include <vulkan/vulkan.h>

namespace Deako
{

	// forward declare
	class VulkanInstance;
	class VulkanPhysicalDevice;
	class VulkanQueue;

	/// VulkanDevice encapulates a Vulkan logical device and references our VulkanQueue objects
	class VulkanDevice : public Inheritor<Object, VulkanDevice>
	{
	public:
		VulkanDevice(Ref<VulkanInstance> instance, Ref<VulkanPhysicalDevice> physicalDevice);
		~VulkanDevice();

	private:
		VkDevice m_Device;

		Ref<VulkanQueue> m_GraphicsQueue;
		Ref<VulkanQueue> m_PresentQueue;
	};

	DK_TYPE_NAME(VulkanDevice);

}

