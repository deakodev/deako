#pragma once

#include "Deako.h"

#include "Panels/ViewportPanel.h"
#include "Panels/ScenePanel.h"
#include "Panels/PropertiesPanel.h"
#include "Panels/RegistryPanel.h"

#include <imgui/imgui.h>

namespace Deako {

    class EditorLayer : public Layer
    {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void OnUpdate() override;
        virtual void OnEvent(Event& event) override;
        virtual void OnImGuiRender(ImTextureID textureID) override;

    private:
        Scope<ScenePanel> m_ScenePanel;
        Scope<PropertiesPanel> m_PropertiesPanel;
        Scope<RegistryPanel> m_RegistryPanel;
        Scope<ViewportPanel> m_ViewportPanel;

        Ref<EditorContext> m_EditorContext;
    };

}
