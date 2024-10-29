#include <Deako.h>
#include <Deako/Core/EntryPoint.h>

#include "EditorContext.h"
#include "EditorLayer.h"

namespace Deako {

    class DeakoEditor : public Application
    {
    public:
        DeakoEditor(const ApplicationSpecification& spec)
            : Application(spec)
        {
        }

        ~DeakoEditor()
        {
            GetLayerStack().PopOverlay(m_ImGuiLayer.get());
            GetLayerStack().PopLayer(m_EditorLayer.get());
        }

        virtual void PushLayers() override
        {
            m_EditorLayer = CreateScope<EditorLayer>();
            GetLayerStack().PushLayer(m_EditorLayer.get());

            m_ImGuiLayer = CreateScope<ImGuiLayer>();
            GetLayerStack().PushOverlay(m_ImGuiLayer.get());
        }

    private:
        Scope<EditorLayer> m_EditorLayer;
        Scope<ImGuiLayer> m_ImGuiLayer;

    };

    Application& CreateApplication(Deako::CommandLineArgs args)
    {
        ApplicationSpecification specification;
        specification.name = "Deako Editor";
        specification.workingDirectory = "../Deako-Editor";
        specification.commandLineArgs = args;

        return InitApplication(new DeakoEditor(specification)); // Eventually have the option to switch between editor mode and gameplay mode??
    }

}
