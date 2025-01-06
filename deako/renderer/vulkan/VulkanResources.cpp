#include "deako_pch.h"
#include "VulkanResources.h"

#include "VulkanInstance.h"
#include "VulkanDebugMessenger.h"

namespace Deako {

	VulkanResources::VulkanResources()
	{
		m_Instance = VulkanInstance::Create();

		m_Instance->Hello();

		DK_CORE_INFO("Instance size: {}", m_Instance->GetSizeOf());
		DK_CORE_INFO("Instance type: {}", m_Instance->GetTypeName());

		m_DebugMessenger = VulkanDebugMessenger::Create(m_Instance);
	}

}