#include "deako_pch.h"
#include "VulkanResources.h"

#include "VulkanInstance.h"
#include "VulkanDebugMessenger.h"
#include "VulkanSurface.h"
#include "VulkanPhysicalDevice.h"
#include "VulkanDevice.h"

namespace Deako {

	VulkanResources::VulkanResources(Window* window)
	{
		m_Instance = VulkanInstance::Create();

		m_DebugMessenger = VulkanDebugMessenger::Create(m_Instance);

		m_Surface = VulkanSurface::Create(m_Instance, window);

		m_PhysicalDevice = m_Instance->SelectPhysicalDevice(m_Surface);

		m_Device = VulkanDevice::Create(m_Instance, m_PhysicalDevice);


	}

}