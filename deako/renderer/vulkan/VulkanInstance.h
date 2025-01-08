#pragma once

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class VulkanPhysicalDevice;
	class VulkanSurface;

	using VulkanInstanceExtensions = std::vector<const char*>;

	/// Instance encapsulates a VkInstance
	class VulkanInstance : public Inheritor<Object, VulkanInstance>
	{
	public:
		VulkanInstance();
		~VulkanInstance();

		Ref<VulkanPhysicalDevice> SelectPhysicalDevice(Ref<VulkanSurface> surface);
		void GetExtensions(VulkanInstanceExtensions* extensions);

		operator VkInstance() const { return m_Instance; }
		VkInstance Vk() const { return m_Instance; }

		void Hello() { return DK_CORE_INFO("Hello There!"); }

	private:
		VkInstance m_Instance;

		std::vector<Ref<VulkanPhysicalDevice>> m_PhysicalDevices;

		bool m_ValidationsEnabled = true;
	};

	DK_TYPE_NAME(VulkanInstance);

}


