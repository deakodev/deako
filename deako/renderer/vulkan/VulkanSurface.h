#pragma once

#include <vulkan/vulkan.h>

namespace Deako
{

	// forward declare
	class VulkanInstance;
	class VulkanWindow;

	class VulkanSurface : public Inheritor<Object, VulkanSurface>
	{
	public:
		VulkanSurface(Ref<VulkanInstance> instance, Window* window);
		~VulkanSurface();

		operator VkSurfaceKHR() const { return m_Surface; }
		VkSurfaceKHR Vk() const { return m_Surface; }

	private:
		VkSurfaceKHR m_Surface;

		Ref<VulkanInstance> m_Instance;
		Window* m_Window; // not owned
	};

	DK_TYPE_NAME(VulkanSurface);

}


