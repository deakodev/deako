#include "Application.h"
#include "dkpch.h"

#include "Deako/Core/Context.h"
#include "Deako/Core/Window.h"

namespace Deako
{

    DkApplication::DkApplication(DkContext* context, DkApplicationConfig& config)
    {
        DK_CORE_ASSERT(context, "DkApplication requires a DkContext!");
        DK_CORE_ASSERT(!config.workingDirectory.empty(), "DkApplication requires a working directory!");
        DK_CORE_ASSERT(!context->application, "DkApplication already created for DkContext!");

        this->name = config.appName;
        this->workingDirectory = config.workingDirectory;
        this->context = context;
    }

    void DkApplication::Run()
    {
        while (this->running)
        {
            if (context->window->active)
            {
                this->layerStack.OnUpdate();
                context->window->OnUpdate();
            }
        }
    }

}
