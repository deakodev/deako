#pragma once

#include "Deako.h"

namespace Deako {

    class ScenePanel
    {
    public:
        void OnImGuiRender();

    private:
        void DrawEntityNode(EntityHandle entity, bool isNodeSelected);
    };

}
