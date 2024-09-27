#pragma once

extern Deako::Application* Deako::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    Deako::Log::Init();

    auto app = Deako::CreateApplication({ argc, argv });

    Deako::Renderer::Init();

    app->Run();

    Deako::Renderer::Shutdown();

    delete app;
}
