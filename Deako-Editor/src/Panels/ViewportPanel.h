#pragma once

#include "Deako.h"

namespace Deako {

    class ViewportPanel
    {
    public:
        ViewportPanel() = default;
        ViewportPanel(Ref<Scene> scene, Ref<ProjectAssetPool> projectAssetPool);

        void SetContext(Ref<Scene> scene, Ref<ProjectAssetPool> projectAssetPool);

        void OnUpdate();
        void OnImGuiRender(ImTextureID textureID);

    private:
        glm::vec2 m_ViewportSize{ 0.0f, 0.0f };
        bool m_ViewportResize{ false };
        bool m_ViewportFocused{ false };
        bool m_ViewportHovered{ false };

        Ref<Scene> m_SceneContext;
        Ref<ProjectAssetPool> m_ProjectAssetPool;
    };

}
