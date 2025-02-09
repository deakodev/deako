#pragma once

#include "renderer/GraphicsResources.h"

#include <vulkan/vulkan.h>

namespace Deako {

	// forward declare
	class GraphicsResources;
	class VulkanInstance;
	class VulkanSurface;
	class VulkanPhysicalDevice;
	class VulkanDevice;
	class VulkanSwapchain;
	struct SwapchainConfig;

	/// VulkanResources encapsulates all Vulkan objects
	class VulkanResources : public Inheritor<GraphicsResources, VulkanResources>
	{
	public:
		VulkanResources(Window* window);

	private:
		void SetupInstance();
		void SetupSurface();
		void SetupPhysicalDevice();
		void SetupDevice();
		void SetupSwapchain();

		void BuildSwapchain(const SwapchainConfig* config = nullptr);

	private:
		Ref<VulkanInstance> m_Instance;
		Ref<VulkanSurface> m_Surface;
		Ref<VulkanPhysicalDevice> m_PhysicalDevice;
		Ref<VulkanDevice> m_Device;
		Ref<VulkanSwapchain> m_Swapchain;

		Window* m_Window; // not owned
	};

	DK_TYPE_NAME(VulkanResources);

}

