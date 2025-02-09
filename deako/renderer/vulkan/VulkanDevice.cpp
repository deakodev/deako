#include "deako_pch.h"
#include "VulkanDevice.h"

#include "VulkanDebugMessenger.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanQueue.h"

namespace Deako
{

	VulkanDevice::VulkanDevice(Ref<VulkanInstance> instance, Ref<VulkanPhysicalDevice> physicalDevice)
	{
		DK_CORE_ASSERT(physicalDevice, "Did you forget to select a physical device?");

		auto [graphicsFamily, presentFamily] = physicalDevice->GetQueueFamilies();
		std::set<int> queueFamilies = { graphicsFamily, presentFamily };

		float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		for (uint32_t queueFamilyIndex : queueFamilies)
		{
			if (queueFamilyIndex < 0) continue;

			// check if queueFamilyIndex has already been referenced or is unique
			bool uniqueIndex = true;
			for (const auto& previousEntry : queueCreateInfos)
			{
				if (previousEntry.queueFamilyIndex == static_cast<uint32_t>(previousEntry.queueFamilyIndex))
				{
					uniqueIndex = false;
				}
			}

			if (!uniqueIndex) continue; // ignore entry, vulkan doesn't support non unique queueFamily

			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfo.pNext = nullptr;

			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures = {};
		deviceFeatures.samplerAnisotropy = VK_TRUE;
		deviceFeatures.fillModeNonSolid = VK_TRUE;

		VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeatures = {};
		dynamicRenderingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynamicRenderingFeatures.dynamicRendering = VK_TRUE;

		VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features = {};
		sync2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
		sync2Features.synchronization2 = VK_TRUE;
		sync2Features.pNext = &dynamicRenderingFeatures;

		std::vector<const char*> deviceExtensions{};
		deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		/*deviceExtensions.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_MULTIVIEW_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
		deviceExtensions.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);*/

#if defined(VK_USE_PLATFORM_MACOS_MVK)
		enabledExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

		bool deviceExtensionsSupported = physicalDevice->SupportsDeivceExtensions(deviceExtensions);
		DK_CORE_ASSERT(deviceExtensionsSupported, "Physical device does not support required extensions!");

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		//createInfo.pNext = &sync2Features; TODO: add later

		if (deviceExtensions.size() > 0)
		{
			createInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
			createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		}
		else
		{
			createInfo.enabledExtensionCount = 0;
			createInfo.ppEnabledExtensionNames = nullptr;
		}

		VK_CHECK(vkCreateDevice(physicalDevice->Vk(), &createInfo, nullptr, &m_Device));

		VkQueue graphicsQueue, presentQueue;
		vkGetDeviceQueue(m_Device, graphicsFamily, 0, &graphicsQueue);
		vkGetDeviceQueue(m_Device, presentFamily, 0, &presentQueue);

		m_GraphicsQueue = CreateRef<VulkanQueue>(graphicsQueue);
		m_PresentQueue = CreateRef<VulkanQueue>(presentQueue);
	}

	VulkanDevice::~VulkanDevice()
	{
		if (m_Device)
		{
			vkDestroyDevice(m_Device, nullptr);
			DK_CORE_INFO("VulkanDevice destroyed!");
		}
	}

	void VulkanDevice::WaitIdle() const
	{
		vkDeviceWaitIdle(m_Device);
	}

}