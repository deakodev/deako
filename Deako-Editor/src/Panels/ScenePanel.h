#pragma once

#include "Deako.h"

namespace Deako {

    class ScenePanel
    {
    public:
        ScenePanel() = default;
        ScenePanel(const Ref<Scene>& context);

        void OnImGuiRender();

        void SetContext(const Ref<Scene>& context);

        Entity GetSelectedEntity() const { return m_SelectionContext; }

    private:
        void DrawEntityNode(Entity entity);
        void DrawComponents(Entity entity);

    private:
        Ref<Scene> m_Context;
        Entity m_SelectionContext;
    };

}
