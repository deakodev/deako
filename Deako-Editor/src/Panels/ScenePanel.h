#pragma once

#include "Deako.h"
#include "../EditorContext.h"

namespace Deako {

    class ScenePanel
    {
    public:
        ScenePanel(Ref<EditorContext> editorContext);

        void OnImGuiRender();

    private:
        bool DrawEntityNode(Ref<Entity> entity);

        Ref<EditorContext> m_EditorContext;
    };

}
