#include "Application.h"
#include "Deak/Events/ApplicationEvent.h"
#include "Deak/Log.h"

namespace Deak
{

    Application::Application() {}
    Application::~Application() {}

    void Application::Run()
    {
        WindowResizeEvent e(1280, 720);
        DK_TRACE(e);

        while (true);
    }

}
