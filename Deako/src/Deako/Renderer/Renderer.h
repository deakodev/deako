#pragma once

#include "Deako/Renderer/EditorCamera.h"



namespace Deako {

    class Renderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render();
    };

}
