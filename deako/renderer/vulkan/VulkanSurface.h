#pragma once

#include "app/deako_window.h"

#include <vulkan/vulkan.h>

namespace Deako
{

	// forward declare
	class VulkanInstance;
	class VulkanWindow;

	/// VulkanSurface encapsulates a Vulkan surface
	class VulkanSurface : public Inheritor<Object, VulkanSurface>
	{
	public:
		VulkanSurface(Window* window, Ref<VulkanInstance> instance);
		~VulkanSurface();

		operator VkSurfaceKHR() const { return m_Surface; }
		VkSurfaceKHR Vk() const { return m_Surface; }

	private:
		VkSurfaceKHR m_Surface;

		Ref<VulkanInstance> m_Instance;
	};

	DK_TYPE_NAME(VulkanSurface);

}


