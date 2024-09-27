#pragma once

#include <glm/glm.hpp>

namespace Deako {

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene();
        static void EndScene();

        static void Invalidate();
    };

}
