#pragma once

#ifdef DK_PLATFORM_MAC

extern Deak::Application* Deak::CreateApplication();

int main(int argc, char** argv)
{
    Deak::Log::Init();
    DZ_CORE_WARN("Initialized Log!");
    [[maybe_unused]] int a = 5;
    DZ_INFO("Hello! var={0}", 5);

    auto app = Deak::CreateApplication();
    app->Run();
    delete app;
}

#endif
