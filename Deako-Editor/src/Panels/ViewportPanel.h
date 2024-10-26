#pragma once

#include "Deako.h"
#include "../EditorContext.h"

#include <ImGuizmo.h>

namespace Deako {

    class ViewportPanel
    {
    public:
        ViewportPanel(Ref<EditorContext> editorContext, Ref<EditorCamera> editorCamera);

        void OnUpdate();
        void OnImGuiRender();

        void TransformWithGizmo();

        void SetGizmoOperation(ImGuizmo::OPERATION operation) { m_GizmoOperation = (int)operation; };

    private:
        glm::vec2 m_ViewportOrigin{ 0.0f, 0.0f };
        glm::vec2 m_ViewportSize{ 0.0f, 0.0f };
        bool m_ViewportResize{ false };
        bool m_ViewportFocused{ false };
        bool m_ViewportHovered{ false };

        Ref<EditorContext> m_EditorContext;
        Ref<EditorCamera> m_EditorCamera;

        int m_GizmoOperation = -1;
    };

}
