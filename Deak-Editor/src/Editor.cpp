#include "Deak.h"
#include "Deak/Core/EntryPoint.h"

#include "EditorLayer.h"

namespace Deak {

    class DeakEditor : public Application
    {
    public:
        DeakEditor()
            : Application("Deak Editor")
        {
            PushLayer(new EditorLayer());
        }

        ~DeakEditor()
        {
        }
    };

    Application* CreateApplication()
    {
        return new DeakEditor();
    }

}
