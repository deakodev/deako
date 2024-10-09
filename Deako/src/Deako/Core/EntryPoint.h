#pragma once

extern Deako::Application* Deako::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    Deako::Log::Init();

    Deako::Application* deakoEditor = Deako::CreateApplication({ argc, argv });

    Deako::Renderer::Init();

    deakoEditor->PushLayers();
    deakoEditor->Run();

    Deako::Renderer::Shutdown();

    delete deakoEditor;
}
