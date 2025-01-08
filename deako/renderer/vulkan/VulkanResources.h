#pragma once

#include "renderer/GraphicsResources.h"

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class GraphicsResources;
	class VulkanInstance;
	class VulkanDebugMessenger;
	class VulkanSurface;
	class VulkanPhysicalDevice;
	class VulkanDevice;

	/// VulkanResources encapsulates all Vulkan objects
	class VulkanResources : public Inheritor<GraphicsResources, VulkanResources>
	{
	public:
		VulkanResources(Window* window);

	private:
		Ref<VulkanInstance> m_Instance;
		Ref<VulkanDebugMessenger> m_DebugMessenger;
		Ref<VulkanSurface> m_Surface;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice> m_Device;
	};

	DK_TYPE_NAME(VulkanResources);

}

