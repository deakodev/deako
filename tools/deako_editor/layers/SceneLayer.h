#pragma once

#include "deako.h"

//#include "Panels/ViewportPanel.h"
//#include "Panels/ScenePanel.h"
//#include "Panels/PropertiesPanel.h"
//#include "Panels/RegistryPanel.h"
//#include "Panels/DebugPanel.h"

//#include <imgui/imgui.h>

namespace Editor {

	class SceneLayer : public Deako::Layer
	{
	public:
		SceneLayer(const char* name);
		virtual ~SceneLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate() override;
		virtual void OnEvent(Deako::Event& event) override;
		virtual void OnImGuiRender() override;

		bool OnKeyPressed(Deako::KeyPressedEvent& event);

	private:
		/*Ref<EditorCamera> m_EditorCamera;

		Scope<ScenePanel> m_ScenePanel;
		Scope<PropertiesPanel> m_PropertiesPanel;
		Scope<RegistryPanel> m_RegistryPanel;
		Scope<ViewportPanel> m_ViewportPanel;
		Scope<DebugPanel> m_DebugPanel;*/
	};

}

