#pragma once

namespace Deako {

	// forward declare
	struct Context;
	class Window;
	class GraphicsResources;

	enum class GraphicsApi { Vulkan = 0, DirectX = 1 };

	class Renderer
	{
	public:
		Renderer(Context* context, Window* window, GraphicsApi api = GraphicsApi::Vulkan);
		~Renderer();

	private:
		Context* m_Context; // parent, not owned
		Window* m_Window; // not owned

		GraphicsApi m_GraphicsApi;
		Ref<GraphicsResources> m_GraphicsResources;
	};


}

