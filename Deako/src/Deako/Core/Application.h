#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Core/Window.h"
#include "Deako/Core/LayerStack.h"

#include "Deako/Event/Event.h"
#include "Deako/Event/WindowEvent.h"
#include "Deako/ImGui/ImGuiLayer.h"

int main(int argc, char** argv);

namespace Deako {

    struct ApplicationCommandLineArgs
    {
        int count = 0;
        char** args = nullptr;

        const char* operator[](int index) const
        {
            DK_CORE_ASSERT(index < count);
            return args[index];
        }
    };

    struct ApplicationSpecification
    {
        std::string name = "Deako Application";
        std::string workingDirectory;
        ApplicationCommandLineArgs commandLineArgs;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& specification);
        virtual ~Application();

        void Close() { m_Running = false; }

        void OnEvent(Event& event);

        static Application& Get() { return *s_Instance; }
        const ApplicationSpecification& GetSpecification() const { return m_Specification; }

        Window& GetWindow() { return *m_Window; }
        LayerStack& GetLayerStack() { return m_LayerStack; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();

        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowResize(WindowResizeEvent& event);
        bool OnWindowMinimized(WindowMinimizedEvent& event);
        bool OnWindowRestored(WindowRestoredEvent& event);

    private:
        ApplicationSpecification m_Specification;

        Scope<Window> m_Window;
        LayerStack m_LayerStack;

        bool m_Running = true;
        bool m_Minimized = false;

        static Application* s_Instance;
    };

    // To be defined client side
    Application* CreateApplication(ApplicationCommandLineArgs args);

}
