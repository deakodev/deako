#include <Deako.h>
#include <Deako/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace Deako {

    class DeakoEditor : public Application
    {
    public:
        DeakoEditor(const ApplicationSpecification& spec)
            : Application(spec)
        {
            m_EditorLayer = CreateScope<EditorLayer>();
            GetLayerStack().PushLayer(m_EditorLayer.get());

            m_ImGuiLayer = CreateScope<ImGuiLayer>();
            GetLayerStack().PushOverlay(m_ImGuiLayer.get());
        }

        ~DeakoEditor()
        {
            GetLayerStack().PopOverlay(m_ImGuiLayer.get());
            GetLayerStack().PopLayer(m_EditorLayer.get());
        }

    private:
        Scope<EditorLayer> m_EditorLayer;
        Scope<ImGuiLayer> m_ImGuiLayer;

    };

    Application* CreateApplication(Deako::ApplicationCommandLineArgs args)
    {
        ApplicationSpecification spec;
        spec.name = "Deako Editor";
        spec.workingDirectory = "../Deako-Editor";
        spec.commandLineArgs = args;

        return new DeakoEditor(spec);
    }

}
