#pragma once

extern Deako::Application& Deako::CreateApplication(Deako::CommandLineArgs args);

int main(int argc, char** argv)
{
    Deako::Log::Init();

    Deako::Application& deakoEditor = Deako::CreateApplication({ argc, argv });

    Deako::Input::Init();
    Deako::Renderer::Init();

    deakoEditor.PushLayers();
    deakoEditor.Run();

    Deako::Renderer::Shutdown();

    Deako::DestroyApplication();
}
