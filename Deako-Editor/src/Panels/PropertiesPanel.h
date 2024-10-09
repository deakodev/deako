#pragma once

#include "Deako.h"
#include "../EditorContext.h"

namespace Deako {

    class PropertiesPanel
    {
    public:
        PropertiesPanel(Ref<EditorContext> editorContext);

        void OnImGuiRender();

    private:
        template<typename T, typename UIFunction>
        void DrawComponent(const std::string& name, UIFunction uiFunction);
        void DrawComponents();

    private:
        Ref<EditorContext> m_EditorContext;
    };

}
