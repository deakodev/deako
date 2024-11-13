#pragma once

extern Deako::DkContext& Deako::CreateContext(DkContextFlags flags);
extern void Deako::DestroyContext();

int main(int argc, char** argv)
{
    Deako::DkLogger::Init();

    Deako::DkContext& deako = Deako::CreateContext();

    deako.application->Run();

    Deako::DestroyContext();
}

