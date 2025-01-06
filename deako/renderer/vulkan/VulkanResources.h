#pragma once

#include "renderer/GraphicsResources.h"

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class GraphicsResources;
	class VulkanInstance;
	class VulkanDebugMessenger;

	/// VulkanResources encapsulates all Vulkan objects
	class VulkanResources : public Inheritor<GraphicsResources, VulkanResources>
	{
	public:
		VulkanResources();

	private:
		Ref<VulkanInstance> m_Instance;
		Ref<VulkanDebugMessenger> m_DebugMessenger;
	};

	DK_TYPE_NAME(VulkanResources);

}

