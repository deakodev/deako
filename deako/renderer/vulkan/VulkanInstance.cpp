#include "deako_pch.h"
#include "VulkanInstance.h"

#include "VulkanPhysicalDevice.h"
#include "VulkanDebugMessenger.h"
#include "VulkanSurface.h"

#include <GLFW/glfw3.h>

namespace Deako {

	VulkanInstance::VulkanInstance()
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "deako_app";
		appInfo.pEngineName = "deako_engine";
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;

#if defined(VK_USE_PLATFORM_MACOS_MVK)
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

		// Instance extensions
		std::vector<const char*> instanceExtensions;
		GetExtensions(&instanceExtensions);

		if (instanceExtensions.size() > 0)
		{
			createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
			createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		}
		else
		{
			createInfo.enabledExtensionCount = 0;
			createInfo.ppEnabledExtensionNames = nullptr;
		}

		// Validation layers
		if (m_ValidationsEnabled)
		{
			const char* validationLayer = "VK_LAYER_KHRONOS_validation";
			createInfo.enabledLayerCount = 1;
			createInfo.ppEnabledLayerNames = &validationLayer;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_Instance));

		if (m_ValidationsEnabled)
		{
			m_DebugMessenger = VulkanDebugMessenger::Create(m_Instance);
		}

		// Physical devices
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
		DK_CORE_ASSERT(deviceCount, "No devices are supported on this platform!");

		std::vector<VkPhysicalDevice> devices{ deviceCount };
		vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

		for (VkPhysicalDevice device : devices)
		{
			m_PhysicalDevices.emplace_back(VulkanPhysicalDevice::Create(device));
		}
	}

	VulkanInstance::~VulkanInstance()
	{
		m_PhysicalDevices.clear();

		if (m_DebugMessenger)
		{
			m_DebugMessenger.reset();
		}

		if (m_Instance)
		{
			vkDestroyInstance(m_Instance, nullptr);
		}

		DK_CORE_INFO("VulkanInstance destroyed!");
	}

	Ref<VulkanPhysicalDevice> VulkanInstance::SelectPhysicalDevice(Ref<VulkanSurface> surface)
	{
		DK_CORE_ASSERT(!m_PhysicalDevices.empty(), "No physical devices found!");

		Ref<VulkanPhysicalDevice> suitableDevice = nullptr;
		for (auto& device : m_PhysicalDevices)
		{
			auto [graphicsFamily, presentFamily] = device->GetQueueFamilies(surface);

			if (graphicsFamily >= 0 && presentFamily >= 0)
			{
				if (device->IsDiscrete())
				{
					DK_CORE_INFO("Selected physical device: {}", device->GetName());
					return device;
				}
				else
				{
					suitableDevice = device;
				}
			}
		}

		DK_CORE_ASSERT(suitableDevice, "No suitable physical device found!");
		DK_CORE_INFO("Selected physical device: {}", suitableDevice->GetName());

		return suitableDevice;
	}

	void VulkanInstance::GetExtensions(VulkanInstanceExtensions* extensions)
	{
		uint32_t requiredExtensionCount = 0;
		const char** requiredExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);

		for (uint32_t i = 0; i < requiredExtensionCount; i++)
			extensions->emplace_back(requiredExtensions[i]);

#if defined(VK_USE_PLATFORM_MACOS_MVK)
		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
		extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
#endif

		if (m_ValidationsEnabled)
		{
			extensions->emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		// check if required instance extensions are supported
		uint32_t supportedExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, nullptr);
		std::vector<VkExtensionProperties> supportedExtensions{ supportedExtensionCount };
		vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount, supportedExtensions.data());

		for (const char* extension : *extensions)
		{
			bool supported = false;
			for (const auto& supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0)
				{
					supported = true; break;
				}
			}
			DK_CORE_ASSERT(supported, "Required instance extensions not supported!");
		}
	}

}
