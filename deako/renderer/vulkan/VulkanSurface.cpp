#include "deako_pch.h"
#include "VulkanSurface.h"

#include "VulkanInstance.h"
#include "VulkanDebugMessenger.h"

#include <GLFW/glfw3.h>

namespace Deako {

	VulkanSurface::VulkanSurface(Window* window, Ref<VulkanInstance> instance)
	{
		m_Instance = instance;

		VK_CHECK(glfwCreateWindowSurface(m_Instance->Vk(), window->GetGlfwWindow(), nullptr, &m_Surface));
	}

	VulkanSurface::~VulkanSurface()
	{
		if (m_Surface)
		{
			vkDestroySurfaceKHR(m_Instance->Vk(), m_Surface, nullptr);
		}

		DK_CORE_INFO("VulkanSurface destroyed!");
	}

}