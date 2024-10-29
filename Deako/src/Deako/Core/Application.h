#pragma once

#include "Deako/Core/Base.h"
#include "Deako/Core/Window.h"
#include "Deako/Core/LayerStack.h"

#include "Deako/Event/Event.h"
#include "Deako/Event/WindowEvent.h"

int main(int argc, char** argv);

namespace Deako {

    struct CommandLineArgs
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
        CommandLineArgs commandLineArgs;
    };

    class Application
    {
    public:
        Application(const ApplicationSpecification& specification);
        virtual ~Application() = default;

        void Close() { m_Running = false; }

        void OnEvent(Event& event);

        const ApplicationSpecification& GetSpecification() const { return m_Specification; }

        virtual void PushLayers() = 0;

        Window& GetWindow() { return *m_Window; }
        LayerStack& GetLayerStack() { return m_LayerStack; }

    private:
        friend int ::main(int argc, char** argv);
        void Run();

        bool OnWindowClose(WindowCloseEvent& event);
        bool OnWindowMinimized(WindowMinimizedEvent& event);
        bool OnWindowRestored(WindowRestoredEvent& event);

    private:
        ApplicationSpecification m_Specification;

        Ref<Window> m_Window;
        LayerStack m_LayerStack;

        bool m_Running = true;
        bool m_Minimized = false;
    };

    Application& CreateApplication(CommandLineArgs args); // To be defined client side

    Application& InitApplication(Application* application); // Sets the editor (or in the future, the game) as the application
    void DestroyApplication();
    Application& GetApplication();

}
