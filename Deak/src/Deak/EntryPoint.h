#pragma once

#ifdef DK_PLATFORM_MAC

extern Deak::Application* Deak::CreateApplication();

int main(int argc, char** argv)
{
    Deak::Log::Init();
    DK_CORE_WARN("Initialized Log!");
    int a = 5;
    DK_INFO("Hello! var={0}", a);

    auto app = Deak::CreateApplication();
    app->Run();
    delete app;
}

#endif
