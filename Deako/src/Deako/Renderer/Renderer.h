#pragma once

#include "Deako/Renderer/EditorCamera.h"

#include <glm/glm.hpp>

namespace Deako {

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene();
        static void EndScene();
    };

}
