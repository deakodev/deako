#pragma once

#include "Deako.h"
#include "../EditorContext.h"

namespace Deako {

    class ViewportPanel
    {
    public:
        ViewportPanel(Ref<EditorContext> editorContext);

        void OnUpdate();
        void OnImGuiRender(ImTextureID textureID);

    private:
        glm::vec2 m_ViewportSize{ 0.0f, 0.0f };
        bool m_ViewportResize{ false };
        bool m_ViewportFocused{ false };
        bool m_ViewportHovered{ false };

        Ref<EditorContext> m_EditorContext;
    };

}
