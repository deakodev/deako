#pragma once

#include "Deako.h"

namespace Deako {

    class ScenePanel
    {
    public:
        ScenePanel() = default;
        ScenePanel(Ref<Scene> scene, Ref<ProjectAssetPool> projectAssetPool);

        void OnImGuiRender();

        void SetContext(Ref<Scene> scene, Ref<ProjectAssetPool> projectAssetPool);

        Entity GetSelectedEntity() const { return m_SelectionContext; }

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

    private:
        Ref<Scene> m_SceneContext;
        Ref<ProjectAssetPool> m_ProjectAssetPool;
        Entity m_SelectionContext;
    };

}
