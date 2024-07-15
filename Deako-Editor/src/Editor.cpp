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
        return new DeakoEditor();
    }

}
