#include <Deako.h>
#include <Deako/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Deako {

    class DeakoEditor : public Application
    {
    public:
        DeakoEditor()
            : Application("Deako Editor")
        {
            PushLayer(new EditorLayer());
        }

        ~DeakoEditor()
        {
        }
    };

    Application* CreateApplication()
    {
        AssetManager::SetModelPath("/models/viking_room.obj");
        AssetManager::SetTexturePath("/textures/viking_room.png");
        return new DeakoEditor();
    }

}
