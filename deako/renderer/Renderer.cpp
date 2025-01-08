#include "deako_pch.h"
#include "Renderer.h"

#include "vulkan/VulkanResources.h"

namespace Deako {

	Renderer::Renderer(Context* context, Window* window, GraphicsApi api)
		: m_Context(context), m_Window(window), m_GraphicsApi(api)
	{
		switch (m_GraphicsApi)
		{
			case GraphicsApi::Vulkan:
			{
				Ref<VulkanResources> resources = VulkanResources::Create(m_Window);
				m_GraphicsResources = std::dynamic_pointer_cast<GraphicsResources>(resources);
				break;
			}
			case GraphicsApi::DirectX:
				DK_CORE_ASSERT(false, "GraphicsApi::DirectX not supported!");
				break;
			default:
				DK_CORE_ASSERT(false, "Unknown GraphicsApi!");
				break;
		};
	}

	Renderer::~Renderer()
	{
	}

}