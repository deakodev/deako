#include "GuiLayer.h"

namespace Editor {

	GuiLayer::GuiLayer(const char* name)
		: Deako::Layer(name)
	{
		DK_TRACE("gui layer created");
	}

	void GuiLayer::OnAttach()
	{
	}

	void GuiLayer::OnDetach()
	{
	}

	void GuiLayer::OnUpdate()
	{
		//DK_TRACE("gui layer updated");
	}

	void GuiLayer::OnEvent(Deako::Event& event)
	{
	}

	void GuiLayer::OnImGuiRender()
	{
	}

	bool GuiLayer::OnKeyPressed(Deako::KeyPressedEvent& event)
	{
		return false;
	}

}

