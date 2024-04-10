#pragma once

#ifdef DK_PLATFORM_MAC

extern Deak::Application* Deak::CreateApplication();

int main(int argc, char** argv)
{
    Deak::Log::Init();

    // DK_PROFILE_BEGIN("Startup", "Sandbox/debug/DeakProfile-Startup.json");
    auto app = Deak::CreateApplication();
    // DK_PROFILE_END();

    // DK_PROFILE_BEGIN("Runtime", "Sandbox/debug/DeakProfile-Runtime.json");
    app->Run();
    // DK_PROFILE_END();

    // DK_PROFILE_BEGIN("Shutdown", "Sandbox/debug/DeakProfile-Shutdown.json");
    delete app;
    // DK_PROFILE_END();
}

#endif
