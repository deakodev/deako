#pragma once

extern Deak::Application* Deak::CreateApplication();

int main(int argc, char** argv)
{
    std::cout << "Deak Engine\n";
    auto app = Deak::CreateApplication();
    app->Run();

    delete app;
}
