#include "deako_pch.h"
#include "VulkanSurface.h"

#include "VulkanInstance.h"
#include "VulkanDebugMessenger.h"

#include <GLFW/glfw3.h>

namespace Deako {

	VulkanSurface::VulkanSurface(Ref<VulkanInstance> instance, Window* window)
	{
		m_Instance = instance;
		m_Window = window;

		VK_CHECK(glfwCreateWindowSurface(m_Instance->Vk(), m_Window->GetGlfwWindow(), nullptr, &m_Surface));
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