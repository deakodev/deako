#include <Deako.h>
#include <Deako/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Deako {

    class DeakoEditor : public DkApplication
    {
    public:
        DeakoEditor(DkContext* context, DkApplicationConfig& config)
            : DkApplication(context, config)
        {
            this->layerStack.PushLayer(m_EditorLayer.get());
            this->layerStack.PushOverlay(m_ImGuiLayer.get());
        }

        ~DeakoEditor()
        {
        }

        private:
         Scope<EditorLayer> m_EditorLayer = CreateScope<EditorLayer>();
         Scope<ImGuiLayer> m_ImGuiLayer = CreateScope<ImGuiLayer>();
    };

    Scope<DkWindow> ConfigureWindow(DkContext* context)
    {
        DkWindowConfig config;
        config.windowName = "Deako Editor";
        config.size = { 1600, 900 };

        return CreateScope<DkWindow>(context, config);
    }

    Scope<DkApplication> ConfigureApplication(DkContext* context)
    {
        DkApplicationConfig config;
        config.appName = "Deako Editor";
        config.workingDirectory = "../Deako-Editor";

        return CreateScope<DeakoEditor>(context, config);
    }

}
 