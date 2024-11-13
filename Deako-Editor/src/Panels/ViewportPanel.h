#pragma once

#include "Deako.h"

#include <ImGuizmo.h>

namespace Deako {

    class ViewportPanel
    {
    public:
        void OnUpdate();
        void OnImGuiRender();

        void TransformWithGizmo(EntityHandle handle);

        void SetGizmoOperation(ImGuizmo::OPERATION operation) { m_GizmoOperation = (int)operation; };

    private:
        DkVec2 m_ViewportOrigin{ 0.0f, 0.0f };
        DkVec2 m_ViewportSize{ 0.0f, 0.0f };
        bool m_ViewportResize{ false };
        bool m_ViewportFocused{ false };
        bool m_ViewportHovered{ false };

        int m_GizmoOperation = -1;
    };

}
