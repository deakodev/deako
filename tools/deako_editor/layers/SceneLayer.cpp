#include "SceneLayer.h"

namespace Editor {

	SceneLayer::SceneLayer(const char* name)
		: Deako::Layer(name)
	{
		DK_TRACE("scene layer created");
	}

	void SceneLayer::OnAttach()
	{
	}

	void SceneLayer::OnDetach()
	{
	}

	void SceneLayer::OnUpdate()
	{
		//DK_TRACE("scene layer updated");
	}

	void SceneLayer::OnEvent(Deako::Event& event)
	{
	}

	void SceneLayer::OnImGuiRender()
	{
	}

	bool SceneLayer::OnKeyPressed(Deako::KeyPressedEvent& event)
	{
		return false;
	}

}
