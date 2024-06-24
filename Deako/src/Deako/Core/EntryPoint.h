#pragma once

extern Deako::Application* Deako::CreateApplication();

int main(int argc, char** argv)
{
    Deako::Log::Init();

    auto app = Deako::CreateApplication();
    app->Run();

    delete app;
}
