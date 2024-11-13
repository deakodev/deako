#pragma once

#include "Deako/Core/LayerStack.h"
#include "Deako/Event/Event.h"

#include <filesystem>

namespace Deako {

    struct DkContext;

    struct DkApplicationConfig
    {
        const char* appName = "Deako App";
        std::filesystem::path workingDirectory;
    };

    struct DkApplication
    {
        const char* name;
        std::filesystem::path workingDirectory;
        LayerStack layerStack;

        DkContext* context; // not owned, parent context 

        bool running{ true };

        DkApplication(DkContext* context, DkApplicationConfig& config);

        void Run();
        void Shutdown() { running = false; }
    };

    Scope<DkApplication> ConfigureApplication(DkContext* context);  // defined/configured on the client side

}
