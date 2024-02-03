#pragma once

#ifdef DK_PLATFORM_MAC

extern Deak::Application* Deak::CreateApplication();

int main(int argc, char** argv)
{
    auto app = Deak::CreateApplication();
    app->Run();
    delete app;
}

#endif
