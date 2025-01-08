#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class VulkanSurface;

	/// PhysicalDevice encapsulates a VkPhysicalDevice
	class VulkanPhysicalDevice : public Inheritor<Object, VulkanPhysicalDevice>
	{
	public:
		VulkanPhysicalDevice(VkPhysicalDevice device);
		~VulkanPhysicalDevice();

		bool SupportsDeivceExtensions(const std::vector<const char*>& extensions);
		bool IsDiscrete() const { return m_Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU; }

		std::pair<int, int> GetQueueFamilies(Ref<VulkanSurface> surface);
		std::pair<int, int> GetQueueFamilies() { return { m_GraphicsFamily, m_PresentFamily }; }
		const std::vector<VkQueueFamilyProperties>& GetQueueFamilyProperties() const { return m_QueueFamilyProperties; }
		const char* GetName() const { return m_Properties.deviceName; }

		operator VkPhysicalDevice() const { return m_Device; }
		VkPhysicalDevice Vk() const { return m_Device; }

	private:
		VkPhysicalDevice m_Device;

		VkPhysicalDeviceFeatures m_Features;
		VkPhysicalDeviceProperties m_Properties;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;

		int m_GraphicsFamily = -1;
		int m_PresentFamily = -1;
	};

	DK_TYPE_NAME(VulkanPhysicalDevice);

}

