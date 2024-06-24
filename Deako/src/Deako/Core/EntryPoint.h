#pragma once

extern Deako::Application* Deako::CreateApplication();

int main(int argc, char** argv)
{
    auto app = Deako::CreateApplication();
    app->Run();

    delete app;
}
